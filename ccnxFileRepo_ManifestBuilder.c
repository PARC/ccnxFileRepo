/*
 * Copyright (c) 2015-2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
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
 * @author <#gscott#>, Computing Science Laboratory, PARC
 * @copyright (c) 2015-2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <ccnx/common/ccnx_Manifest.h>

#include <parc/algol/parc_FileChunker.h>
#include <parc/algol/parc_LinkedList.h>

#include <ccnx/transport/common/transport_MetaMessage.h>

#include "ccnxFileRepo_ManifestBuilder.h"

#include <stdio.h>

struct ccnx_manifest_builder {
    int chunkSize;
};

static bool
_ccnxManifestBuilder_Destructor(CCNxManifestBuilder **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a CCNxManifestBuilder pointer.");
    return true;
}

parcObject_ImplementAcquire(ccnxManifestBuilder, CCNxManifestBuilder);
parcObject_ImplementRelease(ccnxManifestBuilder, CCNxManifestBuilder);
parcObject_Override(CCNxManifestBuilder, PARCObject,
                    .destructor = (PARCObjectDestructor *) _ccnxManifestBuilder_Destructor,
                    .copy = (PARCObjectCopy *) ccnxManifestBuilder_Copy,
                    .toString = (PARCObjectToString *) ccnxManifestBuilder_ToString,
                    .equals = (PARCObjectEquals *) ccnxManifestBuilder_Equals,
                    .compare = (PARCObjectCompare *) ccnxManifestBuilder_Compare,
                    .hashCode = (PARCObjectHashCode *) ccnxManifestBuilder_HashCode,
                    .toJSON = (PARCObjectToJSON *) ccnxManifestBuilder_ToJSON);

void
ccnxManifestBuilder_AssertValid(const CCNxManifestBuilder *instance)
{
    assertTrue(ccnxManifestBuilder_IsValid(instance),
               "CCNxManifestBuilder is not valid.");
}

static PARCBuffer *
_ccnxManifestBuilder_ComputeMessageHash(CCNxMetaMessage *message)
{
    PARCBuffer *wireFormatBuffer = ccnxMetaMessage_CreateWireFormatBuffer(message, NULL);
    CCNxMetaMessage *msg = ccnxMetaMessage_CreateFromWireFormatBuffer(wireFormatBuffer);
    CCNxWireFormatMessageInterface *interface = ccnxWireFormatMessageInterface_GetInterface(msg);
    PARCCryptoHash *hash = interface->computeContentObjectHash(msg);

    PARCBuffer *digest = parcBuffer_Acquire(parcCryptoHash_GetDigest(hash));
    parcCryptoHash_Release(&hash);
    parcBuffer_Release(&wireFormatBuffer);

    return digest;
}

CCNxManifestBuilder *
ccnxManifestBuilder_Create()
{
    CCNxManifestBuilder *result = parcObject_CreateInstance(CCNxManifestBuilder);

    if (result != NULL) {
        result->chunkSize = 4096; // default chunk size
    }

    return result;
}

PARCLinkedList *
ccnxManifestBuilder_BuildSkewedManifest(const CCNxManifestBuilder *builder, PARCChunker *chunker, const CCNxName *name)
{
    PARCLinkedList *chunkList = parcLinkedList_Create();
    CCNxManifestHashGroup *group = ccnxManifestHashGroup_Create();
    PARCIterator *itr = parcChunker_ReverseIterator(chunker);

    // Create a hasher for the application data.
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARC_HASH_SHA256);
    parcCryptoHasher_Init(hasher);

    // Initialize the per-HashGroup metadata values
    size_t applicationDataSize = 0;
    size_t blockSize = parcChunker_GetChunkSize(chunker);
    size_t entrySize = 0;

    while (parcIterator_HasNext(itr)) {
        PARCBuffer *chunk = (PARCBuffer *) parcIterator_Next(itr);

        // Update metadata based on this chunk
        parcCryptoHasher_UpdateBuffer(hasher, chunk);
        size_t nextChunkSize = parcBuffer_Remaining(chunk);
        applicationDataSize += nextChunkSize;
        entrySize += nextChunkSize;

        // Add this ContentObject to the list of HashGroups
        CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, chunk);
        CCNxMetaMessage *metaContent = ccnxMetaMessage_CreateFromContentObject(contentObject);
        parcLinkedList_Append(chunkList, metaContent);

        // Add this ContentObject to the running HashGroup
        PARCBuffer *digest = _ccnxManifestBuilder_ComputeMessageHash(metaContent);
        ccnxManifestHashGroup_PrependPointer(group, CCNxManifestHashGroupPointerType_Data, digest);
        parcBuffer_Release(&digest);

        // Check to see if the HashGroup is full
        if (ccnxManifestHashGroup_IsFull(group)) {
            // Set the HashGroup Metadata
            ccnxManifestHashGroup_SetBlockSize(group, blockSize);
            ccnxManifestHashGroup_SetEntrySize(group, entrySize);
            ccnxManifestHashGroup_SetDataSize(group, entrySize);

            // Reset the HashGroup metadata variables for the next round
            entrySize = 0;

            // Add the HashGroup to a parent manifest
            CCNxManifest *root = ccnxManifest_Create(name);
            ccnxManifest_AddHashGroup(root, group);
            parcLinkedList_Append(chunkList, root);

            CCNxMetaMessage *metaManifest = ccnxMetaMessage_CreateFromManifest(root);
            PARCBuffer *digest = _ccnxManifestBuilder_ComputeMessageHash(metaManifest);
            ccnxMetaMessage_Release(&metaManifest);

            CCNxManifestHashGroup *newGroup = ccnxManifestHashGroup_Create();
            ccnxManifestHashGroup_AppendPointer(newGroup, CCNxManifestHashGroupPointerType_Manifest, digest);
            ccnxManifestHashGroup_Release(&group);
            parcBuffer_Release(&digest);

            group = ccnxManifestHashGroup_Acquire(newGroup);
            ccnxManifestHashGroup_Release(&newGroup);
        }
    }

    // Finalize the overall application data digest
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    PARCBuffer *digest = parcCryptoHash_GetDigest(hash);

    // Add the root metadata to the final HashGroup
    ccnxManifestHashGroup_SetDataSize(group, applicationDataSize);
    ccnxManifestHashGroup_SetOverallDataDigest(group, digest);
    parcCryptoHash_Release(&hash);
    parcCryptoHasher_Release(&hasher);

    // Add the HashGroup to the root manifest and return the result.
    CCNxManifest *manifest = ccnxManifest_Create(name);
    ccnxManifest_AddHashGroup(manifest, group);
    ccnxManifestHashGroup_Release(&group);

    parcLinkedList_Append(chunkList, manifest);
    ccnxManifest_Release(&manifest);

    return chunkList;
}

int
ccnxManifestBuilder_Compare(const CCNxManifestBuilder *instance, const CCNxManifestBuilder *other)
{
    int result = 0;

    assertTrue(ccnxManifestBuilder_IsValid(instance), "Expected a valid CCNxManifestBuilder instance.");
    assertTrue(ccnxManifestBuilder_IsValid(other), "Expected a valid CCNxManifestBuilder instance.");

    if (instance->chunkSize < other->chunkSize) {
        return -1;
    } else if (instance->chunkSize > other->chunkSize) {
        return 1;
    } else {
        return 0;
    }

    return result;
}

CCNxManifestBuilder *
ccnxManifestBuilder_Copy(const CCNxManifestBuilder *original)
{
    CCNxManifestBuilder *result = ccnxManifestBuilder_Create();
    return result;
}

void
ccnxManifestBuilder_Display(const CCNxManifestBuilder *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "CCNxManifestBuilder@%p { chunkSize = %zu }", instance->chunkSize);
}

bool
ccnxManifestBuilder_Equals(const CCNxManifestBuilder *x, const CCNxManifestBuilder *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->chunkSize == y->chunkSize) {
            result = true;
        }
    }

    return result;
}

PARCHashCode
ccnxManifestBuilder_HashCode(const CCNxManifestBuilder *instance)
{
    PARCHashCode result = instance->chunkSize;
    return result;
}

bool
ccnxManifestBuilder_IsValid(const CCNxManifestBuilder *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
ccnxManifestBuilder_ToJSON(const CCNxManifestBuilder *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
        PARCJSONValue *value = parcJSONValue_CreateFromInteger(instance->chunkSize);
        PARCBuffer *entryName = parcBuffer_AllocateCString("chunkSize");
        PARCJSONPair *pair = parcJSONPair_Create(entryName, value);
        parcBuffer_Release(&entryName);
        parcJSON_AddPair(result, pair);
    }

    return result;
}

char *
ccnxManifestBuilder_ToString(const CCNxManifestBuilder *instance)
{
    PARCJSON *json = ccnxManifestBuilder_ToJSON(instance);
    char *result = parcJSON_ToString(json);
    parcJSON_Release(&json);
    return result;
}
