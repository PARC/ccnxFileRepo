/*
 * Copyright (c) 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Patent rights are not granted under this agreement. Patent rights are
 *       available under FRAND terms.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX or PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * @author Christopher A. Wood, Palo Alto Research Center (Xerox PARC)
 * @copyright 2016, Xerox Corporation (Xerox)and Palo Alto Research Center (PARC).  All rights reserved.
 */
#include <LongBow/runtime.h>

#include <stdio.h>
#include <unistd.h>

#include <parc/algol/parc_FileChunker.h>
#include <parc/algol/parc_Chunker.h>

#include <parc/algol/parc_RandomAccessFile.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/algol/parc_LinkedList.h>

#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_Manifest.h>
#include <parc/algol/parc_Object.h>

#include <ccnx/transport/common/transport_MetaMessage.h>

#include "ccnxFileRepo_Cache.h"
#include "ccnxFileRepo_ManifestFetcher.h"

struct ccnx_manifest_fetcher_state {
    CCNxManifest *root;
    size_t hashGroupIndex;
    PARCIterator *digestIterator;
};

typedef struct ccnx_manifest_fetcher_state _FetcherState;

static bool
_ccnxFileRepoManifestFetcherState_Destructor(_FetcherState **statePtr)
{
    _FetcherState *state = *statePtr;

    ccnxManifest_Release(&state->root);
    if (state->digestIterator != NULL) {
        parcIterator_Release(&state->digestIterator);
    }

    return true;
}

parcObject_Override(_FetcherState, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxFileRepoManifestFetcherState_Destructor);

parcObject_ImplementAcquire(_ccnxFileRepoManifestFetcherState, _FetcherState);
parcObject_ImplementRelease(_ccnxFileRepoManifestFetcherState, _FetcherState);

static _FetcherState *
_ccnxFileRepoManifestFetcherState_Create(CCNxManifest *manifest)
{
    _FetcherState *state = parcObject_CreateInstance(_FetcherState);
    if (state != NULL) {
        state->root = ccnxManifest_Acquire(manifest);
        state->digestIterator = NULL;
        state->hashGroupIndex = 0;
    }
    return state;
}

struct ccnx_manifest_fetcher {
    CCNxPortal *portal;
    PARCLinkedList *stateList;
    const CCNxName *locator;

    // Fetching data
    size_t blockSize;

    // log
    PARCLog *log;

    // Manifest fetch state
    PARCBuffer *prevState;
};

/**
 * Create a PARCLog instance to log the request trace information.
 */
static PARCLog *
_ccnxFileRepoManifestFetcher_CreateLogger(void)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create("localhost", "ccnxFileRepoManifestFetcher", NULL, reporter);
    parcLogReporter_Release(&reporter);

    parcLog_SetLevel(log, PARCLogLevel_Info);
    return log;
}

static bool
_ccnxFileRepoManifestFetcher_Destructor(CCNxFileRepoManifestFetcher **fetcherPtr)
{
    CCNxFileRepoManifestFetcher *fetcher = *fetcherPtr;

    ccnxPortal_Release(&fetcher->portal);
    ccnxName_Release((CCNxName **) &fetcher->locator);
    if (fetcher->prevState != NULL) {
        parcBuffer_Release(&fetcher->prevState);
    }

    return true;
}

char *
ccnxFileRepoManifestFetcher_ToString(const CCNxFileRepoManifestFetcher *fetcher)
{
    char *result = NULL;
    char *nameString = ccnxName_ToString(fetcher->locator);
    asprintf(&result, "ccnxFileRepoManifestFetcher for %s\n", nameString);
    parcMemory_Deallocate(&nameString);
    return result;
}

parcObject_Override(CCNxFileRepoManifestFetcher, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxFileRepoManifestFetcher_Destructor,
                    .toString = (PARCObjectToString *) ccnxFileRepoManifestFetcher_ToString);

parcObject_ImplementAcquire(ccnxFileRepoManifestFetcher, CCNxFileRepoManifestFetcher);
parcObject_ImplementRelease(ccnxFileRepoManifestFetcher, CCNxFileRepoManifestFetcher);

CCNxFileRepoManifestFetcher *
ccnxFileRepoManifestFetcher_Create(CCNxPortal *portal, CCNxManifest *root)
{
    CCNxFileRepoManifestFetcher *fetcher = parcObject_CreateInstance(CCNxFileRepoManifestFetcher);
    if (fetcher != NULL) {
        fetcher->portal = ccnxPortal_Acquire(portal);

        fetcher->stateList = parcLinkedList_Create();
        _FetcherState *state = _ccnxFileRepoManifestFetcherState_Create(root);
        parcLinkedList_Append(fetcher->stateList, state);
        _ccnxFileRepoManifestFetcherState_Release(&state);

        fetcher->blockSize = ccnxManifestHashGroup_GetBlockSize(ccnxManifest_GetHashGroupByIndex(root, 0));

        fetcher->locator = ccnxManifest_GetName(root);
        fetcher->log = _ccnxFileRepoManifestFetcher_CreateLogger();
        fetcher->prevState = NULL;
    }
    return fetcher;
}

static PARCBuffer *
_ccnxFileRepoManifestFetcher_GetNextPointer(CCNxFileRepoManifestFetcher *fetcher)
{
    if (parcLinkedList_Size(fetcher->stateList) == 0) {
        return false;
    }

    _FetcherState *state = parcLinkedList_GetLast(fetcher->stateList);
    CCNxManifest *root = state->root;

    if (state->digestIterator == NULL) {
        CCNxManifestHashGroup *group = ccnxManifest_GetHashGroupByIndex(root, state->hashGroupIndex);
        state->digestIterator = ccnxManifestHashGroup_Iterator(group);
    }

    if (parcIterator_HasNext(state->digestIterator)) {
        return (PARCBuffer *) ccnxManifestHashGroupPointer_GetDigest(parcIterator_Next(state->digestIterator));
    } else {
        parcIterator_Release(&state->digestIterator);
        parcLinkedList_RemoveLast(fetcher->stateList);
        return _ccnxFileRepoManifestFetcher_GetNextPointer(fetcher);
    }
}

static CCNxMetaMessage *
_ccnxFileRepoManifestFetcher_FetchData(CCNxFileRepoManifestFetcher *fetcher, PARCBuffer *hashDigest)
{
    CCNxInterest *interest = ccnxInterest_Create(fetcher->locator, 0, NULL, hashDigest);
    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    char *digestString = parcBuffer_ToHexString(hashDigest);
    // parcLog_Info(fetcher->log, "Digest request: %s", digestString);
    parcMemory_Deallocate(&digestString);

    if (ccnxPortal_Send(fetcher->portal, message, CCNxStackTimeout_Never)) {
        CCNxMetaMessage *response = ccnxPortal_Receive(fetcher->portal, CCNxStackTimeout_Never);
        return response;
    }

    return NULL;
}

bool
ccnxFileRepoManifestFetcher_FillBuffer(CCNxFileRepoManifestFetcher *fetcher, PARCBuffer *buffer)
{
    if (fetcher->prevState != NULL) {
        parcBuffer_PutBuffer(buffer, fetcher->prevState);
        parcBuffer_Release(&fetcher->prevState);
    }

    while (parcBuffer_Remaining(buffer)) {
        PARCBuffer *hashDigest = _ccnxFileRepoManifestFetcher_GetNextPointer(fetcher);
        if (hashDigest != NULL) {
            CCNxMetaMessage *response = _ccnxFileRepoManifestFetcher_FetchData(fetcher, hashDigest);
            if (response != NULL) {
                if (ccnxMetaMessage_IsManifest(response)) {
                    CCNxManifest *child = (CCNxManifest *) response;

                    // Reset the root manifest for the fetcher
                    _FetcherState *newState = _ccnxFileRepoManifestFetcherState_Create(child);
                    newState->hashGroupIndex = 0;
                    newState->digestIterator = NULL;
                    parcLinkedList_Append(fetcher->stateList, newState);

                    return ccnxFileRepoManifestFetcher_FillBuffer(fetcher, buffer);
                } else if (ccnxMetaMessage_IsContentObject(response)) {
                    CCNxContentObject *contentObject = (CCNxContentObject *) response;
                    PARCBuffer *childContent = ccnxContentObject_GetPayload(contentObject);

                    // Ensure we can fit the payload into the buffer
                    // If not, store this payload as state for the next fetch and break out
                    // of this request loop
                    if (parcBuffer_Remaining(buffer) >= parcBuffer_Remaining(childContent)) {
                        parcBuffer_PutBuffer(buffer, childContent);
                    } else {
                        fetcher->prevState = parcBuffer_Acquire(childContent);
                        return false;
                    }
                }

                ccnxMetaMessage_Release(&response);
            }
        } else {
            return true;
        }
    }

    return false;
}
