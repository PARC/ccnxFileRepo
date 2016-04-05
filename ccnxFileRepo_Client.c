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
#include <stdio.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>

#include <parc/security/parc_Security.h>

#include <parc/algol/parc_File.h>
#include <parc/algol/parc_RandomAccessFile.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_FileOutputStream.h>

#include <parc/logging/parc_Log.h>
#include <parc/logging/parc_LogReporterFile.h>

#include "ccnxFileRepo_Common.h"
#include "ccnxFileRepo_ManifestFetcher.h"

/**
 * Create a new CCNxPortalFactory instance using a randomly generated identity saved to
 * the specified keystore.
 *
 * @return A new CCNxPortalFactory instance which must eventually be released by calling ccnxPortalFactory_Release().
 */
static CCNxPortalFactory *
_setupConsumerPortalFactory(void)
{
    const char *keystoreName = "client.keystore";
    const char *keystorePassword = "keystore_password";
    const char *subjectName = "ccnxFileRepo_Client";

    return ccnxFileRepoCommon_SetupPortalFactory(keystoreName, keystorePassword, subjectName);
}

/**
 * Create a PARCLog instance to log the request trace information.
 */
static PARCLog *
_ccnxFileRepoClient_CreateLogger(void)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *output = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *reporter = parcLogReporterFile_Create(output);
    parcOutputStream_Release(&output);

    PARCLog *log = parcLog_Create("localhost", "ccnxFileRepoClient", NULL, reporter);
    parcLogReporter_Release(&reporter);

    parcLog_SetLevel(log, PARCLogLevel_Info);
    return log;
}

/**
 * Write the input buffer to the specified file.
 *
 * @param [in] outFile Name of the file to which the buffer will be written.
 * @param [in] data A `PARCBuffer` instance which stores the data to be written.
 */
static void
_ccnxFileRepoClient_AppendBufferToFile(char *outFile, PARCBuffer *data, size_t offset)
{
    PARCFile *out = parcFile_Create(outFile);
    parcFile_CreateNewFile(out);

    PARCRandomAccessFile *fout = parcRandomAccessFile_Open(out);
    parcRandomAccessFile_Seek(fout, offset, PARCRandomAccessFilePosition_Start);
    parcRandomAccessFile_Write(fout, data);
    parcRandomAccessFile_Close(fout);

    parcRandomAccessFile_Release(&fout);
    parcFile_Release(&out);
}

/**
 * Run the consumer to fetch the specified file. Save it to disk once transferred.
 *
 * @param [in] target Name of the content to request.
 * @param [in] outFile Name of the file to which the buffer will be written.
 */
static int
_ccnxFileRepoClient_Run(char *target, char *outFile)
{
    parcSecurity_Init();

    PARCLog *log = _ccnxFileRepoClient_CreateLogger();

    CCNxPortalFactory *factory = _setupConsumerPortalFactory();
    CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Message);
    assertNotNull(portal, "Expected a non-null CCNxPortal pointer.");

    CCNxName *name = ccnxName_CreateFromCString(target);
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    if (ccnxPortal_Send(portal, message, CCNxStackTimeout_Never)) {
        while (ccnxPortal_IsError(portal) == false) {
            CCNxMetaMessage *response = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
            if (response != NULL) {
                if (ccnxMetaMessage_IsManifest(response)) {
                    parcLog_Info(log, "Received root manifest. Beginning to retrieve the content.");

                    // Extract the manifest and instantiate a new fetcher for it
                    CCNxManifest *root = ccnxMetaMessage_GetManifest(response);
                    CCNxFileRepoManifestFetcher *fetcher = ccnxFileRepoManifestFetcher_Create(portal, root);

                    // Initialize the file offset and I/O buffer
                    size_t fileOffset = 0;
                    PARCBuffer *chunkBuffer = parcBuffer_Allocate(ccnxFileRepoCommon_ClientBufferSize);

                    // Start reading from the manifest until done
                    bool done = false;
                    while (!done) {
                        // Reset the buffer information
                        parcBuffer_SetPosition(chunkBuffer, 0);
                        parcBuffer_SetLimit(chunkBuffer, ccnxFileRepoCommon_ClientBufferSize);

                        // Fill the buffer with data from the manifest
                        done = ccnxFileRepoManifestFetcher_FillBuffer(fetcher, chunkBuffer);
                        parcBuffer_Flip(chunkBuffer);

                        // Write the buffer to the file
                        size_t totalSize = parcBuffer_Remaining(chunkBuffer);
                        _ccnxFileRepoClient_AppendBufferToFile(outFile, chunkBuffer, fileOffset);
                        fileOffset += totalSize;

                        // Flip the buffer back around for writing
                        parcBuffer_Flip(chunkBuffer);
                    }
                    parcBuffer_Release(&chunkBuffer);

                    break;
                } else if (ccnxMetaMessage_IsContentObject(response)) {
                    parcLog_Info(log, "Received a content object. Dump the payload and exit.");
                    CCNxContentObject *contentObject = ccnxMetaMessage_GetContentObject(response);
                    PARCBuffer *payload = ccnxContentObject_GetPayload(contentObject);
                    _ccnxFileRepoClient_AppendBufferToFile(outFile, payload, 0);
                    break;
                }
            }
            ccnxMetaMessage_Release(&response);
        }
    }

    ccnxPortal_Release(&portal);
    ccnxPortalFactory_Release(&factory);

    parcSecurity_Fini();
    return 0;
}

/**
 * Display an explanation of arguments accepted by this program.
 *
 * @param [in] programName The name of this program.
 */
static void
_ccnxFileRepoClient_DisplayUsage(const char *programName)
{
    printf("\n%s, %s\n\n", ccnxFileRepoCommon_ProgramName, programName);
    printf("This example file transfer application showcases how a Manifest can be created from a file\n");
    printf("stored in a repository, and served upon request from a consumer.\n");
    printf("\n");
    printf("Usage: %s [-h] <data name> <output name>\n", programName);
    printf("\n");
    printf("   e.g. %s ccnx:/producer/file output.bin\n", programName);
    printf("\n");
    printf("  'data name': the name of the content to request\n");
    printf("  'output name': the file in which the content will be stored\n");
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
        _ccnxFileRepoClient_DisplayUsage(argv[0]);
    }

    if (shouldExit) {
        exit(status);
    }

    if (commandArgCount == 2) {
        status = _ccnxFileRepoClient_Run(commandArgs[0], commandArgs[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        status = EXIT_FAILURE;
        _ccnxFileRepoClient_DisplayUsage(argv[0]);
    }

    exit(status);
}
