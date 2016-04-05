CCNx 1.0 Single File Manifest Transfer
=================

An example program that uses manifests to transfer a single file from the producer
to the consumer.

[CCNx Single File Manifest Transfer main page](https://github.com/PARC/ccnxFileRepo)   
[CCNx.org](https://www.ccnx.org/)

This is the CCNx Single File Manifest Transfer program, a simple example of how to build
manifests from data (files) and recursively fetch them from a client. It includes a set
of programs, with source code, to serve a single file with a manifest and retrieve
said file. The objective is to demonstrate *one* way in which manifests can be built
and used for transport. It is not intended to be the optimal way to transfer data
using manifests. The topic of transport protocols and their interaction with manifests
is a topic of ongoing research and development.

After building, this application consists of 2 programs:

* `ccnxFileRepo_Server`: Serves files out of a directory.
* `ccnxFIleRepo_Client`: Lists and retrieves files from the server.

REQUIREMENTS
------------

This application needs the Distillery CCNx distribution installed on the system.
Please install the [CCNx Distillery](  https://github.com/PARC/CCNx_Distillery) by downloading
it from GitHub, [here]( https://github.com/PARC/CCNx_Distillery), and building it
according to the instructions there.

Building and Running
--------------------

To run the tutorial programs you will need a CCN forwarder (metis or athena) running.
We'll use athena for this example, but either would work.

Note: Metis does not support nameless object support.

Start by running `athena`, then the `ccnxFileRepo_Server` (to serve a single file)
and then the `ccnxFileRepo_Client` to retrieve said file. It is recommended that you run
the `athena`, `ccnxFileRepo_Server` and `ccnxFileRepo_Client` in different windows.

Compiling the application:

1. Set the CCNX_HOME environment variable to the location of your Distillery build. In zsh, for example,
it might look like this:
`export CCNX_HOME=/path/to/CCNx_Distillery/usr`

2. If you ran 'make all' when you built the CCNx_Distillery distribution, you should already have
   the binaries in your $CCNX_HOME/bin directory.

   If they are not there, you can run `make ccnxFileRepo` from the CCNx_Distillery directory.

3. At this point, the compiled binaries for `ccnxFileRepo_Client` and the
`ccnxFileRepo_Server` should be built and exist in $CCNX_HOME/bin

4. Start a forwarder. Do ONE of the following:

   4a. Start the CCNx forwarder, `metis_daemon`:
    `$CCNX_HOME/bin/metis_daemon --capacity 0 &`

   4b. Start the CCNx forwarder, `athena`:
    `$CCNX_HOME/bin/athena -s 0 &`

5. Run the `ccnxFileRepo_Server`:
  Start the `ccnxFileRepo_Server`, giving it a directory path as an argument.
  `$CCNX_HOME/bin/ccnxFileRepo_Server /path/to/file /path/to/repo/store ccnx:/published/name`

6.  Run the `ccnxFileRepo_Client` to retrieve the file "/path/to/file" from the server.

  `$CCNX_HOME/bin/ccnxFileRepo_Client ccnx:/published/name out.bin `

NOTE: Do not run the `ccnxFileRepo_Client` with the same output file name as the server file. This will cause things to break.

## Notes: ##

- The `ccnxFileRepo_Client` and `ccnxFileRepo_Server` automatically create keystore files in
  their working directory.

- You can experiment with different chunk sizes and client receive buffer sizes by changing the values of
`ccnxFileRepoCommon_ServerChunkSize` and `ccnxFileRepoCommon_ClientBufferSize`, respectively. Both
of these are defined in `ccnxFileRepo_Common.c`.


If you have any problems with the system, please discuss them on the developer
mailing list:  `ccnx@ccnx.org`.  If the problem is not resolved via mailing list
discussion, you can file tickets in the issue tracker.


CONTACT
-------

For any questions please use the CCNx mailing list.  [ccnx@ccnx.org](mailto:ccnx@ccnx.org)


LICENSE
-------

This software is licensed under the PARC Software License. See LICENSE File.
