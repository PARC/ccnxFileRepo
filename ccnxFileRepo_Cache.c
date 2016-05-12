/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */
/**
 * @author Christopher A. Wood, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <LongBow/runtime.h>

#include <stdio.h>
#include <stdlib.h>
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

#include <ccnx/transport/common/transport_MetaMessage.h>

#include "ccnxFileRepo_Cache.h"
#include "ccnxFileRepo_ManifestBuilder.h"

struct ccnx_file_repo_cache {
    PARCLog *log;
    char *directory;
    size_t chunkSize;
};

/**
 * Create a PARCLog instance to log the request trace information.
 */
static PARCLog *
_ccnxFileRepoCache_CreateLogger(void)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create("localhost", "ccnxFileRepoCache", NULL, reporter);
    parcLogReporter_Release(&reporter);

    parcLog_SetLevel(log, PARCLogLevel_Info);
    return log;
}

static bool
_ccnxFileRepoCache_Destructor(CCNxFileRepoCache **repoPtr)
{
    CCNxFileRepoCache *repo = *repoPtr;

    parcMemory_Deallocate(&repo->directory);
    return true;
}

char *
ccnxFileRepoCache_ToString(const CCNxFileRepoCache *repo)
{
    char *result = NULL;
    return result;
}

parcObject_Override(CCNxFileRepoCache, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxFileRepoCache_Destructor,
                    .toString = (PARCObjectToString *) ccnxFileRepoCache_ToString);

parcObject_ImplementAcquire(ccnxFileRepoCache, CCNxFileRepoCache);
parcObject_ImplementRelease(ccnxFileRepoCache, CCNxFileRepoCache);

CCNxFileRepoCache *
ccnxFileRepoCache_Create(char *directory, size_t chunkSize)
{
    CCNxFileRepoCache *repo = parcObject_CreateInstance(CCNxFileRepoCache);
    if (repo != NULL) {
        repo->directory = parcMemory_StringDuplicate(directory, strlen(directory));
        repo->chunkSize = chunkSize;
        repo->log = _ccnxFileRepoCache_CreateLogger();
    }
    return repo;
}

static char *
_ccnxFileRepoCache_JoinPath(CCNxFileRepoCache *repo, char *suffix)
{
    PARCBufferComposer *fileComposer = parcBufferComposer_Create();
    parcBufferComposer_PutString(fileComposer, repo->directory);
    parcBufferComposer_PutString(fileComposer, "/");
    parcBufferComposer_PutString(fileComposer, suffix);
    char *fullName = parcBufferComposer_ToString(fileComposer);
    parcBufferComposer_Release(&fileComposer);
    return fullName;
}

static PARCBuffer *
_ccnxFileRepoCache_SaveToRepo(CCNxFileRepoCache *repo, CCNxMetaMessage *message)
{
    PARCBuffer *buffer = ccnxMetaMessage_CreateWireFormatBuffer(message, NULL);
    CCNxMetaMessage *msg = ccnxMetaMessage_CreateFromWireFormatBuffer(buffer);
    CCNxWireFormatMessageInterface *interface = ccnxWireFormatMessageInterface_GetInterface(msg);
    PARCCryptoHash *hash = interface->computeContentObjectHash(msg);

    PARCBuffer *digest = parcBuffer_Acquire(parcCryptoHash_GetDigest(hash));
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&buffer);

    char *fileName = parcBuffer_ToHexString(digest);
    char *fullName = _ccnxFileRepoCache_JoinPath(repo, fileName);
    parcLog_Info(repo->log, "Saving file: %s", fullName);

    PARCBuffer *wireBuffer = ccnxMetaMessage_CreateWireFormatBuffer(message, NULL);
    PARCFile *file = parcFile_Create(fullName);
    if (!parcFile_Exists(file)) {
        parcFile_CreateNewFile(file);
    }

    PARCRandomAccessFile *raf = parcRandomAccessFile_Open(file);
    parcRandomAccessFile_Write(raf, wireBuffer);

    parcRandomAccessFile_Close(raf);
    parcRandomAccessFile_Release(&raf);

    return digest;
}

PARCBuffer *
ccnxFileRepoCache_CreateWireEncodedMessageWithDigest(CCNxFileRepoCache *repo, PARCBuffer *digest)
{
    char *fileName = parcBuffer_ToHexString(digest);
    char *fullName = _ccnxFileRepoCache_JoinPath(repo, fileName);

    PARCBuffer *result = NULL;

    PARCFile *file = parcFile_Create(fullName);
    if (parcFile_Exists(file)) {
        size_t fileSize = parcFile_GetFileSize(file);
        PARCRandomAccessFile *fhandle = parcRandomAccessFile_Open(file);
        result = parcBuffer_Allocate(fileSize);
        parcRandomAccessFile_Read(fhandle, result);
        parcBuffer_Flip(result);

        parcRandomAccessFile_Close(fhandle);
        parcRandomAccessFile_Release(&fhandle);
    }
    parcMemory_Deallocate(&fileName);
    parcMemory_Deallocate(&fullName);

    parcFile_Release(&file);
    return result;
}

static CCNxManifest *
_ccnxFileRepoCache_BuildFile(CCNxFileRepoCache *cache, CCNxName *name, PARCFile *file)
{
    PARCFileChunker *fileChunker = parcFileChunker_Create(file, cache->chunkSize);
    PARCChunker *chunker = parcChunker_Create(fileChunker, PARCFileChunkerAsChunker);
    parcFileChunker_Release(&fileChunker);

    CCNxManifestBuilder *builder = ccnxManifestBuilder_Create();
    PARCLinkedList *chunks = ccnxManifestBuilder_BuildSkewedManifest(builder, chunker, name);

    PARCIterator *itr = parcLinkedList_CreateIterator(chunks);
    while (parcIterator_HasNext(itr)) {
        CCNxMetaMessage *metaMessage = (CCNxMetaMessage *) parcIterator_Next(itr);
        _ccnxFileRepoCache_SaveToRepo(cache, metaMessage);
    }
    parcIterator_Release(&itr);
    ccnxManifestBuilder_Release(&builder);
    parcChunker_Release(&chunker);

    CCNxManifest *root = ccnxMetaMessage_Acquire(parcLinkedList_GetLast(chunks));
    parcLinkedList_Release(&chunks);

    return root;
}

CCNxManifest *
ccnxFileRepoCache_LoadFile(CCNxFileRepoCache *cache, CCNxName *name, PARCFile *file)
{
    return _ccnxFileRepoCache_BuildFile(cache, name, file);
}
