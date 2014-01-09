Description
---------------------

ChildKiller is a C++ Windows tool that, when given a command,
executes it by spawning a "cmd" child process and when receiving a TerminateProcess,
also kills all child processes _recursively_. Otherwise the tool tries to act
transparent and behave in the same way as the command given.

This overrides the Windows default behavior of letting child processes live when
their parent is killed, which is usually the Linux behavior. Thus when using this
tool on Windows and a same-named command-forwarding linux script, one can achieve
platform-transparent command execution.


Example
---------------------

As an example take an "external tool" in Eclipse, which spawns a gradle process
to build some java project and gradle spawns multiple child "worker" processes.
When hitting the terminate button in Eclipse, on linux-systems, gradle and all workers
are killed recursively, but in Windows, only gradle, but not the workers are killed,
thus being a major annoyance.
When now using ChildKiller and passing the gradle command line to it, a terminate from
Eclipse kills: ChildKiller, the gradle process, and the gradle workers.

Compare the process tree:

Eclipse
|-gradle
  |-worker1
  |-worker2

Terminate Results in:

Eclipse
worker1
worker2

With ChildKiller:

Eclipse
|-ChildKiller
  |-gradle
    |-worker1
    |-worker2

After Terminate:

Eclipse


Implementation
---------------------

ChildKiller uses the Windows "Jobs" API (see source) to create a job
and add itself to it. After this, all newly spawned child processes
are automatically part of that same job.

When a parent process in a job is killed, all child processes in that same job
are also killed, thus implementing the desired behavior.

Enjoy!
Thomas