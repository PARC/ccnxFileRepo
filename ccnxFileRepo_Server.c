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

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>

#include <parc/security/parc_Security.h>
#include <parc/algol/parc_RandomAccessFile.h>

#include "ccnxFileRepo_Common.h"
#include "ccnxFileRepo_Cache.h"

/**
 * Create a new CCNxPortalFactory instance using a randomly generated identity saved to
 * the specified keystore.
 *
 * @return A new CCNxPortalFactory instance which must eventually be released by calling ccnxPortalFactory_Release().
 */
static CCNxPortalFactory *
_setupConsumerPortalFactory(void)
{
    const char *keystoreName = "server.keystore";
    const char *keystorePassword = "keystore_password";
    const char *subjectName = "ccnxFileRepo_Server";

    return ccnxFileRepoCommon_SetupPortalFactory(keystoreName, keystorePassword, subjectName);
}

/**
 * Run a producer that will serve the specified file under the specified content name.
 * The file will be transferred using a Manifest. The repo will create the manifest from
 * the specified file.
 *
 * @param [in] fileName Path to the file to serve.
 * @param [in] repoBase Directory to store the repo.
 * @param [in] contentName Name under which to publish the content.
 */
static int
_runProducer(char *fileName, char *repoBase, char *contentName)
{
    parcSecurity_Init();

    CCNxPortalFactory *factory = _setupConsumerPortalFactory();

    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Message);
    assertNotNull(portal, "Expected a non-null CCNxPortal pointer.");

    // Create the repo and load the first and only file
    CCNxFileRepoCache *cache = ccnxFileRepoCache_Create(repoBase, 4096);
    CCNxName *name = ccnxName_CreateFromCString(contentName);

    PARCFile *file = parcFile_Create(fileName);
    CCNxManifest *manifest = ccnxFileRepoCache_LoadFile(cache, name, file);
    parcFile_Release(&file);

    printf("Published: %s\n", ccnxName_ToString(name));

    // Start listening for requests
    if (ccnxPortal_Listen(portal, name, 365 * 86400, CCNxStackTimeout_Never)) {
        while (true) {
            CCNxMetaMessage *request = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);

            if (request == NULL) {
                break;
            }

            CCNxInterest *interest = ccnxMetaMessage_GetInterest(request);

            if (interest != NULL) {
                CCNxName *interestName = ccnxInterest_GetName(interest);
                if (ccnxName_Equals(interestName, name)) {
                    PARCBuffer *digest = ccnxInterest_GetContentObjectHashRestriction(interest);
                    if (digest != NULL) {
                        PARCBuffer *chunk = ccnxFileRepoCache_CreateWireEncodedMessageWithDigest(cache, digest);
                        if (chunk != NULL) {
                            CCNxMetaMessage *response = ccnxMetaMessage_CreateFromWireFormatBuffer(chunk);
                            if (ccnxPortal_Send(portal, response, CCNxStackTimeout_Never) == false) {
                                fprintf(stderr, "ccnxPortal_Send failed: %d\n", ccnxPortal_GetError(portal));
                            }
                        }
                    } else {
                        CCNxMetaMessage *response = ccnxMetaMessage_CreateFromManifest(manifest);
                        if (ccnxPortal_Send(portal, response, CCNxStackTimeout_Never) == false) {
                            fprintf(stderr, "ccnxPortal_Send failed: %d\n", ccnxPortal_GetError(portal));
                        }
                    }
                }
            }
            ccnxMetaMessage_Release(&request);
        }
    }

    ccnxFileRepoCache_Release(&cache);
    ccnxName_Release(&name);

    return 0;
}

/**
 * Display an explanation of arguments accepted by this program.
 *
 * @param [in] programName The name of this program.
 */
static void
_displayUsage(const char *programName)
{
    printf("\n%s, %s\n\n", ccnxFileRepoCommon_ProgramName, programName);
    printf("This example file transfer application showcases how a Manifest can be created from a file\n");
    printf("stored in a repository, and served upon request from a consumer.\n");
    printf("\n");
    printf("Usage: %s [-h] <file name> <repo path> <content name>\n", programName);
    printf("\n");
    printf("   e.g. %s /path/to/file /path/to/repo ccnx:/producer/file\n", programName);
    printf("\n");
    printf("  'file name': the path of the file to serve\n");
    printf("  'repo path': the directory where the Manifest chunks should be stored\n");
    printf("  'content name': the CCNx name under which the file will be published\n");
    printf("  '-h' will show this help\n\n");
}

int
main(int argc, char *argv[argc])
{
    int status = EXIT_FAILURE;

    char *commandArgs[argc];
    int commandArgCount = 0;
    bool needToShowUsage = false;
    bool shouldExit = false;

    status = ccnxFileRepoCommon_ProcessCommandLineArguments(argc, argv, &commandArgCount, commandArgs,
                                                            &needToShowUsage, &shouldExit);

    if (needToShowUsage) {
        _displayUsage(argv[0]);
    }

    if (shouldExit) {
        exit(status);
    }

    if (commandArgCount == 3) {
        return (_runProducer(commandArgs[0], commandArgs[1], commandArgs[2]) ? EXIT_SUCCESS : EXIT_FAILURE);
    } else {
        status = EXIT_FAILURE;
        _displayUsage(argv[0]);
    }
}
