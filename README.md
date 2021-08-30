![](docs/logo.png)

The **OpenXPC™** project is an independent and source-compatible
reimplementation of Apple's XPC, an IPC mechanismm. It is hosted on the D-Bus
Message Oriented Middleware. OpenXPC is intended principally for Airyx OS, but
is also compatible with other free-software BSDs and GNU+Linux. It is not yet
complete.

### Architecture

*Architecture is subject to change to improve compatibility.* The described
architecture isn't fully implemented yet, but describes the basic plan of
action. 

XPC services run in one of three kinds of domain: the singleton system domain
for services available to all apps and libraries; the per-user session domain,
and the per-app app domain. The first two are directly mapped to the D-Bus
system and session domains. The third, being app-specific, is implemented by
the setting of an environment variable by LaunchServices during the launch of an
app, that variable naming a private folder for that app. Unix domain sockets are
created in that folder bearing the service name of each XPC service, and Libdbus
is used to directly speak the D-Bus protocol over these, without involving D-Bus
Daemon.

xpcd/xpcctl are provisional daemons which provide a subset of the launchd
interfaces. Whether they will be necessary is yet to be determiend.

### Licencing

OpenXPC is free and open source software.
