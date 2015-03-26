

# KEDR and Autotest #

Here we describe how to use KEDR in conjunction with [Autotest](http://autotest.github.com/). We assume that the readers are already familiar with Autotest itself, with how to write tests for it and how to run them.

There are two different use cases considered below:

  * development of the new Autotest-based tests that use KEDR
  * using KEDR in conjunction with the existing Autotest-based tests

## KEDR and new tests ##

KEDR has a [public repository](http://code.google.com/p/kedr/source/checkout), can be installed using standard `configure` (`cmake`) - `make` - `make install` steps and its analysis tools can be controlled from command line. So writing _new_ tests that use KEDR and are managed by Autotest should not be difficult.
In general, such tests could operate as follows:

  * Load KEDR and specify the kernel module to be analyzed (_target module_).
  * Load the target module (or do something else that will make the system load that module).
  * Make requests to the target module: if it is a device driver, operate on the corresponding device; if it is a file system module, mount this file system and do something with it, etc. The test may simply send requests to the module or it may check some functional requirements and so on. In short, do anything you want to with the target kernel module. Meanwhile, KEDR will monitor how the latter works.
  * Unload the target module.
  * Collect and, if necessary, process the reports prepared by KEDR.
  * Unload KEDR.

See ["Getting Started" tutorial](kedr_manual_getting_started.md) for the examples of how to analyze kernel modules with KEDR.

## KEDR and existing tests ##

In this section, we show how to use KEDR as a "profiler" for existing tests. The advantage is that once written, the profiler can be reused for many tests in a particular area. We will use [memory leak detector](http://code.google.com/p/kedr/wiki/kedr_manual_using_kedr#Detecting_Memory_Leaks) ("LeakCheck") provided by KEDR to check the kernel module which is indirectly involved by the user space test.

Note that it is a profiler only from the point of view of Autotest. It just seems to be a technically convenient way to use KEDR with the existing tests without changing these tests. KEDR itself does no performance measurements or other things usually associated with profiling.

### Profiler features ###
The main challenge in using KEDR for a given test is to find out which exactly kernel module is used by the test and how to load and unload this module (KEDR should be loaded before the target module and unloaded after the latter was unloaded).

Because different kernel modules correspond to different subsystems (file systems, video/audio, net, etc.), different actions are needed to unload these modules. For some modules `rmmod` is sufficient, others require additional steps, such as unmounting file systems using these modules or stopping user space applications that involve these modules, etc.

Our profiler will allow to check the modules of the following kinds:

  * modules that can be unloaded using simple `rmmod`
  * modules that require to stop a network interface before `rmmod`
  * modules that first need to unload other modules using the above mechanisms.

Before the test is executed, the profiler starts appropriate components of KEDR (including LeakCheck) for the specified kernel module. If the target module was loaded at that time, it is temporarily unloaded. After the test, the profiler parses the reports generated by LeakCheck and extracts some values from it: number of memory allocations made, number of leaks detected, etc. These values are then stored as "performance values". Finally, the profiler unloads the analyzed module and then stops KEDR.

The profiler expects that KEDR is already installed on the system, and its install path is known. General build and install instructions are in ["Getting Started"](kedr_manual_getting_started.md), there are also [Chromium OS-specific instructions](HowTo_Chromium_OS.md).

### Profiler code ###
Although the example profiler is not very complex, its python code is somewhat long. Here is the code of the KEDR-based profiler that correctly processes unloading of network modules: [kedr\_net\_profiler.py](http://code.google.com/p/kedr/source/browse/snippets/autotest/kedr_net_profiler/kedr_net_profiler.py?repo=aux)

### Applying KEDR-based profiler to the test ###
To use our profiler for the test, write a simple control file for Autotest:

```
AUTHOR = "<your name here>"
NAME = "network_test_with_KEDR"
TIME = "SHORT"
TEST_CATEGORY = "Functional"
TEST_CLASS = "General"
TEST_TYPE = "client"

DOC = """
Run network test with KEDR LeakCheck running as a profiler.
"""

# Dependencies between kernel modules and network interfaces.
module_interface_dependencies = (('forcedeth', 'eth0'),)

job.profilers.add('kedr_net_profiler',
    module_name = 'forcedeth',
    module_interface_dependencies = module_interface_dependencies)
job.run_test('network_Ping')
job.profilers.delete('kedr_net_profiler')
```

This control file means that our `kedr_net_profiler` will be used for test `network_Ping`, that KEDR LeakCheck tool will be run for `forcedeth` module and that this module is used for network interface `eth0`. This dependency between the kernel module and network interface is system-specific and may vary for different OSes and hardware.

Your system may use other kernel modules to manage network instead of 'forcedeth', depending on your hardware ('`atl1*`', '`ath*`', '`e1000`' and many others). Examples of control files suitable for other systems:

```
...
# Dependencies between kernel modules and network interfaces.
module_interface_dependencies = (('atl1e', 'eth0'),)

job.profilers.add('kedr_net_profiler',
    module_name = 'atl1e',
...
```

will use KEDR LeakCheck for `atl1e` module, which is used for network interface `eth0`;

```
...
# Dependencies between kernel modules and network interfaces.
module_interface_dependencies = (('ath5k', 'wlan0'),)

job.profilers.add('kedr_net_profiler',
    module_name = 'ath5k',
...
```

will use KEDR LeakCheck for `ath5k` module, which is used for network interface `wlan0`;

```
...
# Dependencies between kernel modules and network interfaces.
module_interface_dependencies = (('ath5k', 'wlan0'),)

job.profilers.add('kedr_net_profiler',
    module_name = 'cfg80211',
...
```

will use KEDR LeakCheck for `cfg80211` module, the module `ath5k` depending on it is used for network interface `wlan0`.

### Running the test with profiler ###
As usual for Autotest's client tests, the python file with the test should be placed under `client/tests/network_Ping` subdirectory of Autotest and empty `__init__.py` file should be placed along with it. Python file with the profiler should be placed under `client/profilers/kedr_net_profiler` also with empty `__init__.py` file.

Control file with the contents shown above should be stored in a directory of your choice from where it is convenient to run tests: it can be the control file of the profiler itself `client/profilers/kedr_net_profiler/control` or the control file for a separate test (`client/tests/network_test_with_kedr/control`). Then running

```
./server/autoserv
```


or other suitable utility with path to this control file will run the test.

#### Chromium OS-specific information ####
On [Chromium OS](http://www.chromium.org/chromium-os), Autotest is enabled by `--withautotest` parameter to the `./build_packages` command and is installed under `/build/x86-generic/usr/local/autotest`. The test control file should be stored in `client/tests/<test_name>/control` or `client/site_tests/<test_name>/control`. The test is executed using
```
./run_remote_tests.sh --board=${BOARD} --remote=${REMOTE_IP} <test_name>
```


### Directions for profiler improvement ###
The example profiler is of course not universally applicable. Still, its code is designed so as to make it simple to extend its functionality. Some of the directions of future improvement are outlined below.

#### Ability to process kernel modules for other subsystems ####
The profiler allows watching for the modules responsible for file systems, such as fat or ext4, and for the modules that such modules depend on.

#### Automatic detection of module role ####
Instead of hardcoding the dependencies between the kernel modules and subsystems they belong to, these dependencies could be detected automatically using the standard command line utilities and/or sysfs filesystem. Such feature would improve reusability of the test control file on different machines.

#### Automatically choosing a module to check ####
Instead of using the hard-coded name of the kernel module to be analyzed, the profiler could find this module automatically for each particular machine. Such feature would also improve reusability of the test control file on different machines.