

# Customizing and Extending KEDR #

## Using Code Generator to Create Custom Modules ##


To automate creation of multiple modules with simular functionality, KEDR actively uses template-based generation of files. This approach facilitates code reuse as it makes it possible to separate the common parts in the implementation of the modules and the parts specific to each module. This also allows the developer of the new modules to concentrate mostly on the "logic" of what (s)he wants to implement there rather than on writing and debugging "boilerplate" code.



So, to develop a new module this way, it is only necessary to prepare a short definition of what this module is supposed to do in addition to the basic functionality. Creation of the source file(s) for this module will be performed automatically by the "code generator".



Apart from usage within KEDR, this mechanism can also be used for creating custom specialized modules for different purposes: payload modules for call monitoring or fault simulation, fault simulation indicators, etc. This approach to development of custom modules has many advantages:


<ul><li>
<b>fast development</b> - the implementation of a new payload payload module for call monitoring requires, for example, about 10 lines in the "definition" file for the "header" part (it contains the name of the module, the author and the license, etc.), and about 10 lines per target function (description of the arguments and the return value, etc.) the calls to which are to be traced.<br>
</li>
<li>
<b>clear and readable definition files</b> - all features of your module are described in one place, the so called <i>definition file</i> rather than scattered over different files or over one long file. Every line in a "definition" file is self-explanatory.<br>
</li>
<li>
<b>high level of abstraction</b> - when writing a "definition" file, you do not need to care about what file(s) will be generated from it and how exactly any particular feature will be implemented.<br>
</li>
<li>
<b>less error-prone code</b> - if the "definition" file is written correctly, correct code of the module will be generated from it. Most of the lines in that file simply define the names of some entities (variables, types, etc.) that will appear in the generated code. The rare inter-line dependencies as well as code chunk definitions can be easily debugged in the clear and short "definition" file.<br>
</li>
<li>
<b>easier maintenance</b> - if the templates are updated to implement some new basic functionality, to fix errors or for any other reason, it is enough to run the "generator" again to update the code of the modules you have created. The enhancements and fixes will thus automatically propagate to all the modules generated using those templates.<br>
</li>
</ul>

Of course, using the "generator" is not an universal way to extend functionality of the standard KEDR modules. If some functionality is not provided by the templates, it will not be available for the generated modules. You will probably need to implement it manually - or prepare the templates of your own. Still, in many cases it can be very convenient to use the "generator" with the default templates to create modules for KEDR.



Let us now consider the common format of "definition" files.



The "generator" is based on MiST Engine library from [Template2Code project](http://template2code.sourceforge.net) and is very similar to "mist\_gen" example from that project. As a result, the format of definition files accepted by the generator is the same as the format of <i>configuration files</i> accepted by "mist\_gen". The format is fully described [here](http://template2code.sourceforge.net/mist-doc/param.html). The only difference is that a definition file (as well as a configuration file for "mist\_gen") may contain `[group]` keywords that divide the file into blocks.



A definition file is treated as an array of records. The lines that contain only whitespace characters (spaces and tabs) are ignored, so are the lines where the first non-whitespace character is `#`:

```
# The next line is empty, so it will be ignored

    # This line will be ignored too.
```




Lines like

```
<parameter-name> = <value>
```

define a parameter with name `<parameter-name>` and assign the string `<value>` to it. `<parameter-name>` may only contain latin letters, digits, dots, hyphens and underscores. The names are case-sensitive. Whitespace characters surrounding `<parameter-name>` and `<value>` are ignored.




```
# Define parameter with name 'a' and value '135'
a = 135
# Define parameter with name 'b' and value 'some string'
b = some string
# Define parameter with name 'expression' and value '2 + 3 = 5'
expression = 2 + 3 = 5
```




There is a way to define parameter with a long value:

```
# Define parameter with name 'long-string' and value 'string1 string2 string3'
# Note, that leading whitespace characters are ignored.
long-string = string1 \
    string2 \
    string3
```




In addition, parameters with multiline values can be defined too:

```
multi-line-parameter =>>
    line1
    line2
    ...
    lineN
<<
```




The value of `multi-line-parameter` is precisely as follows:

```
    line1
    line2
    ...
    lineN
```




Note that a newline character should immediately follow `>>` delimiter and apart from the delimiter `<<`, there should be no characters on the line (except whitespace characters).


```
# Correct definition of a multiline parameter containing >>
multi-line-parameter =>>
    <<a>>
    <<b>>
    <<
```




The generator only extracts the set of parameters with and  their values from the definition file. The order in which these parameters are listed is not important. For example, the following definition files

```
a = 5
b = 10
```

and

```
b = 10
a = 5
```

have actually the same meaning.



However when several definitions assign values to the same parameter, the parameter becomes multi-valued and the order of the assignments becomes important. Example:




```
a = 5
a = 10
```

This means `a={'5','10'`}, but

```
a = 10
a = 5
```

means `a={'10','5'`}.



Depending on the meaning of the parameter, the difference in the order of its values may be important (e.g. the order of the function parameters is critical), or it may be not (e.g. the order of the target functions descriptions in the file).



As a rule, the order of values of two multi-valued parameters is only significant if these parameters describe one-value attributes of same object:




```
obj.name = object1
obj.description = This is object1
obj.name = object2
obj.description = This is object2
```

This defines `obj.name` as "object1", "object2", `obj.description` as "This is object1", "This is object2".
This may mean there are two object instances with attributes "object1", "This is object1" and "object2", "This is object2"



Let us consider the following definitions where the values of `obj.description` are given in a reverse order.

```
obj.name = object1
obj.description = This is object2
obj.name = object2
obj.description = This is object1
```

This defines `obj.name` as "object1", "object2", `obj.description` as "This is object2", "This is object1". This may mean two object instances with attributes
"object1", "This is object2" and "object2", "This is object1", which is probably not what you want.



A simple way to avoid such confusion with ordering is to define all attributes for one instance first and only then define attributes for another one.



If some object has a non-constant set of attributes (e.g., one of its attributes may have multiple values or one of its attribute is optional), then you cannot define several instances of this object in one definition file. This is because the generator cannot determine which instance each particular value of an attribute belongs to. To address this problem, `[group]` keyword was introduced in the format of definition files. This keyword denotes a new group of definitions that starts just after this keyword and ends before the next occurence of same keyword or at the end of the file.




```
module_name = Arrays
[group]
array.name = array1
array.values = val1
[group]
array.name = array2
array.values = val2
array.values = val3
[group]
array.name = array3
```

There are 3 groups in this file. The first one defines `array.name='array1'` and `array.values='val1'`, the second - `array.name='array2'` and `array.values={'val2', 'val3'`}, third - `array.name='array3'`. Each group can be interpreted as a definition of an array object. The object named `array1` contains one element `val1`, the object named `array2` contains two elements `val2` and `val3`, the object named `array3` contains no elements.



`[group]` keyword does not prevent gathering of all parameter assignments. That is, the "global meaning" of this file is `module_name='Arrays'`, `array.name={'array1', 'array2', 'array3'`} and `array.values={'val1', 'val2', val3'`}. This information will be processed by the generator using one set of templates. But besides that, the information from each group will also be processed using another set of templates. This processing will result in a new multi-valued parameter which values are the results processing of the groups. This parameter is referred to as "block", and so is the set of templates used to  generate it. This parameter can be used at the top level of processing, the set of templates for which is referred to as "document".



As far as payload modules are concerned, "document" templates define the overall layout of the generated source and header files while "block" templates define the parts of the code related to a target function. That is, a `[group]` block corresponds to a target function in this case.



This section has given a brief overview of template-based code generation mechanism used in KEDR. This should be enough though if you would like to write you own "definition" files for custom modules with the templates provided by KEDR. See [MiST Engine Reference Manual](http://template2code.sourceforge.net/mist-doc/index.html) and ["mist\_gen" example](http://template2code.sourceforge.net) if you want to learn more about this way of template-based code generation.


## Writing Custom Payloads for Call Monitoring ##


This section describes how to write a new payload module for call monitoring using a tool provided by KEDR to generate source files from the templates. Common abilities of this tool are described in detail in ["Using Code Generator to Create Custom Modules"](kedr_manual_extend#Using_Code_Generator_to_Create_Custom_Modules.md).



Typical purposes of a custom payload module of this kind could be as follows:


<ul><li>

support call monitoring for the functions for which it is not supported by KEDR "out-of-the-box";<br>
<br>
</li>
<li>

change the set of parameters output to the trace, in case you need something other than the arguments and the return value of the target function to be output.<br>
<br>
</li>
</ul>

The whole infrastructure necessary for building the payload module from the "definition file" is located in `custom_payload_callm` subdirectory in the directory where the examples provided with KEDR are installed (`/usr/local/share/kedr/examples/` by default). Here are its contents:

<ul><li><b><code>payload.data</code></b>

'definition' file to create the payload module</li>
<li><b><code>makefile</code></b>

file for common build infrastructure for <b><code>make</code></b> utility</li>
<li><b><code>Kbuild</code></b>

file for building kernel module from C sources</li>
</ul>
To use all this in development of your payload module, copy the contents of that directory to a directory of your choice.



The first and the main step is to rewrite `payload.data` to reflect definitions of your payload module.



At the global level (i.e. before the first group begins), this file should contain definitions for the following parameters:

<ul><li><b>module.name</b>

string, which will be used as module name inside its source files</li>
<li><b>module.author</b>

author of the module</li>
<li><b>module.license</b>

license for the module</li>
</ul>
In addition, the following parameters may be defined at the global level:

<ul><li><b>header</b>

the code (may be multiline) to be inserted before the definition of target functions. This code usually contains '#include' directives for header files which define target functions and types of its parameters.</li>
</ul>
Example of global section of the `payload.data` file:

```
# This module processes the calls to module_put function.

module.name = payload_callm_module_put
module.author = Andrey Tsyvarev
module.license = GPL

header =>>
#include <linux/module.h>
<<
```




For each [target function](kedr_manual_glossary#Target_function.md) the information about which is to be output to the trace, a group should be prepared in the definition file.
Each group should contain definitions for the following parameters:

<ul><li><b>function.name</b>

name of the target function</li>
<li><b>returnType</b>

return type of the target function if it is not void, otherwise shouldn't be defined at all</li>
<li><b>arg.type</b>

(multi-valued) types of the parameters of the target function, starting with the first one. If the function has no parameters, shouldn't be assigned at all.</li>
<li><b>arg.name</b>

(multi-valued) names of the parameters of the target function, starting with the first one. If the function has no parameters, shouldn't be assigned at all. Parameters of the target function will be accessible via these names in the code.</li>
<li><b>trace.param.name</b>

(multi-valued) variable names, which values will be output to the trace. This variables should be accessible when parameters will be output to the trace.</li>
<li><b>trace.param.type</b>

(multi-valued) types of the values, which will be output into the trace. This types will be used for casting the values of the corresponding variables before they will be output (so, these types may differ from the real types of variables).</li>
<li><b>trace.formatString</b>

format string which is used for printf-like output of values from target function (see parameters "trace.param.name" and "trace.param.type")</li>
</ul>


<blockquote><font><b><u>Important</u></b></font>

Output to the trace is currently supported only for the variables of simple types (i.e. no strings, arrays, structures, etc.). Pointers can be output using <code>"%p"</code> or a similar format. This is due to the limitations of <b><code>kedr_gen</code></b>. In the future versions, these limitations may be removed or at least relaxed.<br>
<br>
</blockquote>


Additionally, the following parameters can be defined at group level:

<ul><li><b>prologue</b>

code (may be multiline) which will be executed before the values are output to the trace. Usually, this code declares variables that will be used in the output.</li>
<li><b>epilogue</b>

code (may be multiline) which will be executed after the values have been output to the trace. If <code>prologue</code> request some resources from the kernel, this is the place where these resources can be released.</li>
</ul>

If the target function does not return void, `ret_val` variable refers to the return value of the function. It may be used as the name of variable to output (arg.name) and in the prologue and epilogue sections.



Example of the group section for module\_put() target function:

```
[group]
    # Name and return type of the target function
    function.name = module_put

    # Names and types of the arguments of the target function
    arg.type = struct module *
    arg.name = m
    
    # The parameters to be output to the trace. 
    trace.param.type = void *
    trace.param.name = m

    # The format string to be used for trace output.
    trace.formatString = "arguments: (%p)"

# End of the group of definitions for module_put().
```

Example of the group section for kmalloc() target function (note the usage of `ret_val` variable as the value of `trace.param.name` parameter):

```
[group]
    # Name and return type of the target function
    function.name = __kmalloc
    returnType = void *

    # Names and types of the arguments of the target function
    arg.type = size_t
    arg.name = size

    arg.type = gfp_t
    arg.name = flags
    
    # The parameters to be output to the trace. 
    trace.param.type = size_t
    trace.param.name = size

    trace.param.type = unsigned int
    trace.param.name = flags

    trace.param.type = void *
    trace.param.name = ret_val

    # The format string to be used for trace output.
    trace.formatString = "arguments: (%zu, %x), result: %p"

# End of the group of definitions for __kmalloc().
```

Example of the group section for kmem\_cache\_alloc() target function (note the contents of prologue parameter):

```
    [group]
    # Name and return type of the target function
    function.name = kmem_cache_alloc
    returnType = void *

    # Names and types of the arguments of the target function
    arg.type = struct kmem_cache *
    arg.name = mc
    
    arg.type = gfp_t
    arg.name = flags
    
    prologue =>>
size_t size = kmem_cache_size(mc);
<<
    # The parameters to be output to the trace. 
    trace.param.type = size_t
    trace.param.name = size

    trace.param.type = unsigned int
    trace.param.name = flags

    trace.param.type = void *
    trace.param.name = ret_val

    # The format string to be used for trace output.
    trace.formatString = "arguments: (%zu, %x), result: %p"

# End of the group of definitions for kmem_cache_alloc().
```




As you can see, kmem\_cache\_alloc() function does not have `size` argument. If we still want to output the size of the requested memory block to the trace, we need to determine it for for use in output to the trace. This is exactly what is done in the prologue code above.



After writing `payload.data` file, you can change the value of module\_name variable in the `makefile` and `Kbuild` according to the one you use as value of "module.name" parameter. In the future, this step may be implemented in the `makefile` itself.



The last step is to run <b><code>make</code></b> utility. This will invoke the code generator tool (<b><code>kedr_gen</code></b>) to create the sources for your payload module, then the module will be built.


## trace.happensBefore Parameter for Call Monitoring ##


There is a parameter that changes order of [target function](kedr_manual_glossary#Target_function.md) call and output of values to the trace:

<ul><li><b>trace.happensBefore</b>

If this parameter is defined (its actual value does not matter), the trace will be output <b>before</b> the target function is called. With this parameter defined, variable <code>ret_val</code> should not be used.</li>
</ul>



The main purpose of using `trace.happensBefore` parameter is collecting correct trace on SMP systems and the like. Suppose, two threads of execution call `mutex_lock` and `mutex_unlock` functions for the same mutex. One of the correct sequences of these calls is:


```
[1]    mutex_lock
[1]    mutex_unlock
[2]    mutex_lock
[2]    mutex_unlock
```


(`[n]` means that the operation is performed by the thread _n_).



So one may expect that same order will be recorded in the trace:


```
1    called_mutex_lock
1    called_mutex_unlock
2    called_mutex_lock
2    called_mutex_unlock
```


Normally, target function is called and then its parameters are output to the trace. So the following order of the instructions is possible:


```
[1]    [call trampoline function for mutex_lock()]
[1]    mutex_lock
[1]    output("called_mutex_lock")
[1]    [trampoline function for mutex_lock() returns]
[1]    [call trampoline function for mutex_unlock]
[1]    mutex_unlock
[2]    [call trampoline function for mutex_lock()]
[2]    mutex_lock
[2]    output("called_mutex_lock")
[2]    [trampoline function for mutex_lock() returns]
[1]    output("called_mutex_unlock")
[1]    [trampoline function for mutex_unlock() returns]
[2]    [call trampoline function for mutex_unlock]
[2]    mutex_unlock
[2]    output("called_mutex_unlock")
[2]    [trampoline function for mutex_unlock() returns]
```


Even though the order of calls to the target functions is correct, these instructions produce the trace that shows an impossible order of the calls:


```
1    called_mutex_lock
2    called_mutex_lock
1    called_mutex_unlock
2    called_mutex_unlock
```


From the kernel's point of view, the calls to `mutex_lock` and `mutex_unlock` are not related to the trace output made by KEDR. So the operations that output trace can be performed in any order, no matter in what order the target functions were called.



To get a correct trace, we need to use `trace.happensBefore` parameter for the target function `mutex_unlock`. At the abstract level, this parameter means "Whenever `mutex_unlock` is called before some other function and nobody enforces this order explicitly, the order should be preserved in the trace".



Note that although `mutex_lock` function must also be called before the corresponding `mutex_unlock`, this order is not affected by `trace.happensBefore` parameter.


```
...
[1]    mutex_lock
...
[1]    mutex_unlock
...
```


The difference is that such order should be enforced **explicitly**, that is, by the user of these functions and thus of the target module. If the target module calls, say, `mutex_lock` strictly before `mutex_unlock`, the trampoline function for `mutex_lock` will return strictly before the one for `mutex_unlock` starts executing. This, in turn, automatically enforces that the corresponding trace records will go in the right order too.



Another example of internal happens-before relationship is the one between `kfree` and `__kmalloc` functions. This relationship reflects the fact that `__kmalloc` cannot return an address that was previously returned by another `__kmalloc` call and was not processed by `kfree`. Using `trace.happensBefore` parameter one can make sure the order of the trace records is correct:


```
    [group]
    # Name and return type of the target function
    function.name = kfree

    # Names and types of the arguments of the target function
    arg.type = void*
    arg.name = p
    
    # The parameters to be output to the trace.
    trace.param.type = void*
    trace.param.name = p

    # Happens-before relationship with kmalloc
    trace.happensBefore = yes

    # The format string to be used for trace output.
    trace.formatString = "arguments: (%p)"

# End of the group of definitions for kfree().
```

<blockquote><font><b><u>Note</u></b></font>

For the trace records to reflect the fact that function <code>A</code> has happens-before ordering with function <code>B</code>, one should define <code>trace.happensBefore</code> parameter for the function <code>A</code> and <b>should not define it</b> for the function <code>B</code>. <code>trace.happensBefore</code> cannot be used to enforce two different orderings for the calls to a single function such as <code>krealloc</code> (which may be modelled as <code>__kmalloc</code> followed by <code>kfree</code>).<br>
<br>
</blockquote>

## Writing Custom Payloads for Fault Simulation ##


This section describes how to write a new payload module for fault simulation using <b><code>kedr_gen</code></b> tool provided by KEDR to generate source files from the templates. Common abilities of <b><code>kedr_gen</code></b> are described in detail in ["Using Code Generator to Create Custom Modules"](kedr_manual_extend#Using_Code_Generator_to_Create_Custom_Modules.md).



Typical purposes of a custom payload module of this kind could be as follows:


<ul><li>

support fault simulation for the functions for which it is not supported by KEDR "out-of-the-box";<br>
<br>
</li>
<li>

provide a different set of variables to be passed to fault simulation indicator (see below) - this can be necessary if you would like to implement <a href='kedr_manual_extend#Writing_Custom_Scenarios_for_Fault_Simulation.md'>custom fault simulation scenarios</a>.<br>
<br>
</li>
</ul>
<blockquote><font><b><u>Note</u></b></font>

Note that the infrastructure provided by KEDR for fault simulation (<a href='kedr_manual_glossary#Fault_simulation_point.md'>points</a>, <a href='kedr_manual_glossary#Fault_simulation_indicator.md'>indicators</a> and the respective control facilities) could be used for other purposes as well. In general, it allows to alter the behaviour of a call made by the target module if the indicator returns nonzero, or allow the target function to do its work normally if the indicator returns 0. The "altered behaviour" is controlled by the user, see the description of <code>fpoint.fault_code</code> parameter below. So, instead of fault simulation, you could implement, say, timeout/sleep injection (i.e., delaying the return from the replacement function which might help with concurrency analysis, etc.) or whatever else you want.<br>
<br>
</blockquote>


The files necessary to build the payload module from the "definition file" are located in `custom_payload_fsim` subdirectory in the directory where the examples provided with KEDR are installed (`/usr/local/share/kedr/examples/` by default). Here are its contents:

<ul><li><b><code>payload.data</code></b>

'definition' file to create the payload module</li>
<li><b><code>makefile</code></b>

file for common build infrastructure for <b><code>make</code></b> utility</li>
<li><b><code>Kbuild</code></b>

file for building kernel module from C sources</li>
</ul>
To use all this in development of your payload module, copy the contents of that directory to a directory of your choice.



The first and the main step is to rewrite `payload.data` to reflect definitions of your payload module.



At the global level (i.e. before the first group begins), this file should contain definitions for the following parameters:

<ul><li><b>module.name</b>

string, which will be used as module name inside its source files</li>
<li><b>module.author</b>

author of the module</li>
<li><b>module.license</b>

license for the module</li>
</ul>
In addition, the following parameters may be defined at the global level:

<ul><li><b>header</b>

the code (may be multiline) to be inserted before the definition of replacement functions. This code usually contains '#include' directives for header files which define target functions and types of its parameters.</li>
</ul>
Example of global section of the `payload.data` file:

```
# This module processes the calls to kstrdup function.

module.name = payload_fsim_kstrdup
module.author = Andrey Tsyvarev
module.license = GPL

header =>>
#include <linux/string.h>
<<
```




For each [target function](kedr_manual_glossary#Target_function.md) to be processed, a group should be prepared in the definition file.
Each group should contain definitions for the following parameters:


<ul><li><b>function.name</b>

name of the target function</li>
<li><b>returnType</b>

return type of the target function if it is not void, otherwise shouldn't be defined at all</li>
<li><b>arg.type</b>

(multi-valued) types of the parameters of the target function, starting with the first one. If the function has no parameters, shouldn't be assigned at all.</li>
<li><b>arg.name</b>

(multi-valued) names of the parameters of the target function, starting with the first one. If the function has no parameters, shouldn't be assigned at all. Parameters of the target function will be accessible via these names in the code.</li>
<li><b>fpoint.fault_code</b>

code (may be multiline) which should be executed instead the call to the target function to simulate failure of the latter. Usually, this code simply sets <code>ret_val</code> variable to a value indicating that a failure has occured. For the caller of the target function, if will look like the target function has returned this value.</li>
<li><b>fpoint.param.name</b>

(multi-valued) names of the variables which values will be passed to the indicator function and may be used to specify the scenario of fault simulation in it. The order of these variables is important, because they will be passed sequentially to the indicator function. Usually, only the parameters of the target function are passed to the indicator.</li>
<li><b>fpoint.param.type</b>

(multi-valued) types of the values that will be passed to the indicator function. These types will be used to properly cast the values before passing to the indicator (so these types may differ from the real types of the variables).</li>
<li><b>fpoint.rename</b>

Instead of using the name of the target function as a  name of the fault simulation point, use the name given by this parameter.</li>
<li><b>fpoint.reuse_point</b>

Instead of creating a new fault simulation point for this function, use the previously defined point with the name given by this parameter.</li>
</ul>

Additionally, the following parameters can be defined at the group level. They are similar to the ones used in the payload modules for call monitoring.


<ul><li><b>prologue</b>

code (may be multiline) which will be inserted at the start of replacement function (before the call to the indicator function which should decide whether need to simulate a failure or not). Usually, this code declares variables that will be passed to the indicator function.</li>
<li><b>epilogue</b>

code (may be multiline) which will be inserted at the end of the replacement function (after  executing target function or simulating its fault). If <code>prologue</code> request some resources from the kernel, this is the place to release these resources.</li>
</ul>

If the target function does not return void, `ret_val` variable should be set in fpoint.fault\_code to the value that the target function would return in case of the particular failure.



Here is an example of the group section for kmalloc target function. Note the definition of `fpoint.fault_code` parameter. Its value is "`ret_val = NULL;`" because '=' characters after the leftmost one have no special meaning and are treated as the part of the value.


```
[group]
    # Name and return type of the target function
    function.name = __kmalloc
    returnType = void *

    # Names and types of the arguments of the target function
    arg.type = size_t
    arg.name = size

    arg.type = gfp_t
    arg.name = flags
    
    # Fault Simulation
    fpoint.param.type = size_t
    fpoint.param.name = size

    fpoint.param.type = gfp_t
    fpoint.param.name = flags

    fpoint.fault_code = ret_val = NULL;

# End of the group of definitions for __kmalloc().
```


Example of the group section for kstrdup() target function:

```
[group]
    # Name and return type of the target function
    function.name = kstrdup
    returnType = char *

    # Names and types of the arguments of the target function
    arg.type = const char *
    arg.name = str
    
    arg.type = gfp_t
    arg.name = flags

    # Calculate length of the string
    prologue = size_t len = strlen(str);

    # Fault Simulation
    fpoint.param.type = size_t
    fpoint.param.name = len

    fpoint.param.type = gfp_t
    fpoint.param.name = flags

    fpoint.fault_code = ret_val = NULL;

# End of the group of definitions for kstrdup().
```




Note the usage of `len` variable for fault simulation in the example above. This value is calculated in the prologue based on the target function parameter `str` and is then used as one of the parameters to be passed to the fault simulation indicator.



It is possible for different replacement functions to share the same indicator function (and, therefore, share the scenario). This is more than simply using the same indicator functions, this is using a single instance of an indicator. These functions may use some data private for each indicator instance. In case of sharing, the data will also be shared.



Sharing of the indicator functions can be useful, for example, for the target functions that are known to use a common mechanism internally (e.g. memory allocator), and you want to simulate a failure of this mechanism.



If, say, function `g` should share the fault simulation scenario with `f`, you should define parameter `fpoint.reuse_point` in the group of function `g` with value `f`. In this case, the group for function `f` should precede the group for function `g`. Example of sharing a fault simulation scenario for `__kmalloc` and `krealloc`:

```
    ...
[group]
    # Name and return type of the target function
    function.name = __kmalloc
    returnType = void *

    # Names and types of the arguments of the target function
    arg.type = size_t
    arg.name = size

    arg.type = gfp_t
    arg.name = flags
    
    # Fault Simulation
    fpoint.param.type = size_t
    fpoint.param.name = size

    fpoint.param.type = gfp_t
    fpoint.param.name = flags

    fpoint.fault_code = ret_val = NULL;
    
# End of the group of definitions for __kmalloc().

[group]
    # Name and return type of the target function
    function.name = krealloc
    returnType = void *

    # Names and types of the arguments of the target function
    arg.type = const void *
    arg.name = p

    arg.type = size_t
    arg.name = size

    arg.type = gfp_t
    arg.name = flags
    
    # Fault Simulation
    fpoint.reuse_point = __kmalloc

    fpoint.param.type = size_t
    fpoint.param.name = size

    fpoint.param.type = gfp_t
    fpoint.param.name = flags

    fpoint.fault_code = ret_val = NULL;

# End of the group of definitions for krealloc().
```

Note that in the group for `krealloc` function, we use the same names and types of the variables intended to be passed to the indicator function, as for `__kmalloc`.



If a fault simulation point is reused, it may be convenient to assign a name to this point that is different from the name of the target function the point was defined for:


```
    ...
[group]
    function.name = __kmalloc
    ...
    fpoint.rename = kmalloc
    ...
# End of the group of definitions for __kmalloc().

[group]
    function.name = krealloc
    ...
    fpoint.reuse_point = kmalloc
    ...
# End of the group of definitions for krealloc().
```


Note, that when reusing a fault simulation point defined with `fpoint.rename` parameter, the name specified there is used (`kmalloc`) rather than the name of the target function the point was defined for (`__kmalloc`).



After writing `payload.data` file, you can change the value of module\_name variable in the `makefile` and `Kbuild` according to the one you use as value of "module.name" parameter. In the future, this step may be implemented in the `makefile` itself.



The last step is to run <b><code>make</code></b> utility. This will invoke the code generator tool (<b><code>kedr_gen</code></b>) to create the sources for your payload module, then the module will be built.


## Writing Custom Scenarios for Fault Simulation ##


The fault simulation scenarios described in ["Fault Simulation"](kedr_manual_using_kedr#Fault_Simulation.md) are configurable and are probably enough for many cases. If they are not, a kernel module implementing a custom [fault simulation indicator](kedr_manual_glossary#Fault_simulation_indicator.md) can be developed. This section describes how do this using <b><code>kedr_gen</code></b> tool provided by KEDR to generate source files from the templates. Common abilities of <b><code>kedr_gen</code></b> are described in detail in ["Using Code Generator to Create Custom Modules"](kedr_manual_extend#Using_Code_Generator_to_Create_Custom_Modules.md)



The whole infrastructure needed to build the module based on the definition file and the templates is located in `custom_indicator_fsim` subdirectory in the directory where the examples provided with KEDR are installed (`/usr/local/share/kedr/examples/` by default). Here are its contents:

<ul><li><b><code>indicator.data</code></b>

'definition' file to create the module that will implement the fault simulation indicator</li>
<li><b><code>makefile</code></b>

file for common build infrastructure for <b><code>make</code></b> utility</li>
<li><b><code>Kbuild</code></b>

file for building kernel module from C sources</li>
<li><b><code>calculator.c</code>, <code>calculator.h</code>, <code>control_file.c</code>, <code>control_file.h</code></b>

additional source and header files that implement some of the indicator's functionality. These files are used for building the module.</li>
</ul>
To use all this in development of your module, copy the contents of that directory to a directory of your choice.



The first and the main step is to rewrite file `indicator.data` to reflect the definitions of your indicator module.



Unlike a payload module for [call monitoring](kedr_manual_extend#Writing_Custom_Payloads_for_Call_Monitoring.md) or [fault simulation](kedr_manual_extend#Writing_Custom_Payloads_for_Fault_Simulation.md) that can process several target functions in a single module, each fault simulation indicator should be implemented in a separate module. So, groups are not used in the definition file for the indicator module, only the global set of parameters is taken into account.



The following parameters should be defined in that file:

<ul><li><b>module.author</b>

author of the module</li>
<li><b>module.license</b>

license for the module</li>
<li><b>indicator.name</b>

name of the indicator, provided by the module. This is the very name that should be used when one applies the indicator to some target function (to be exact, to a <a href='kedr_manual_glossary#Fault_simulation_indicator.md'>fault simulation point</a>).</li>
<li><b>indicator.parameter.type</b>

(multi-valued) types of the values that the indicator function accepts. This is an important part of the indicator and will be described later in detail. This parameter may be assigned no value at all - in this case, the indicator function will accept no parameters. </li>
<li><b>indicator.parameter.name</b>

(multi-valued) names of the values that the indicator function accepts.</li>
<li><b>expression.variable.name</b>

(multi-valued) names of variables that can be used in an expression to set a particular scenario for the indicator (see also <a href='kedr_manual_using_kedr#Fault_Simulation.md'>"Fault Simulation"</a>). The names themselves are by no means bound to the names of variables used in the indicator. The order of values is not important for this parameter. This parameter may even be left undefined as there are other ways to declare expression variables.</li>
<li><b>expression.variable.value</b>

(multi-valued) values of the corresponding expression variables that will be used during the evaluation of the expression (that is, when the indicator function is called). Typically, these values refer to the parameters of the indicator function.</li>
<li><b>expression.variable.pname</b>

(multi-valued) names of the parameters of the indicator function that can be used in an expression to set a particular scenario for the indicator. <code>expression.variable.pname = var_a</code> is equivalent to <code>expression.variable.name = var_a</code> followed by <code>expression.variable.value = var_a</code>. This parameter may even be left undefined as there are other ways to declare expression variables.</li>
</ul>



The main characteristic of a fault simulation indicator is a set of scenarios it can implement. Apart from `pid` parameter that can be used for each generated indicator and simply restricts the "area" of fault simulation, `expression` is the only indicator's parameter, which may affect the fault simulation scenario. An expression that uses only constant integers as arguments may implement simple  scenarios like <i>"always simulate failure"</i> or <i>"never simulate failure"</i>. But if the expression can use variables which may have different values each time the expression is evaluated, the set of supported scenarios increases dramatically.



One type of variable that can be used in the expression is the parameter of the target function. E.g., expression `(size > 100)`, where `size` corresponds to the target function parameter, implements the scenario <i>"simulate failure when <code>size</code> is greater than <code>100</code>"</i>. The only way for the indicator to implement such usage of a target function's parameter is to declare this parameter as parameter of indicator function. So, the corresponding replacement function should pass this parameter to the indicator function whenever it should choose whether it needs to simulate a failure. This behaviour of the indicator is achieved by the following definitions (assume `size` parameter of the target function has type `size_t`):

```
indicator.parameter.type = size_t
indicator.parameter.name = size
```




This fragment only defines that the indicator function itself accepts parameter `size`. To permit using this parameter in the expression, the following definition should be added:


```
expression.variable.pname = size
```


Parameters `expression.variable.name` and `expression.variable.value` may be useful for such cases:

```
...
indicator.parameter.type = const char *
indicator.parameter.name = str
...
# Expression may use variables only with integer values, so we cannot use 
# a string parameter in it.
# But we can use the length of this string as parameter 'len'
expression.variable.name = len
expression.variable.value = strlen(str)
...
```


```
...
indicator.parameter.type = size_t
# Cannot use 'strlen' as name of the parameter, because strlen() is 
# the kernel function.
indicator.parameter.name = len
...
# But here 'strlen' is available - this is not a name of C variable.
expression.variable.name = strlen
# We only need to bind expression variable to its value.
expression.variable.value = len
...
```




However if we declare that the indicator function accepts parameter `size` of type `size_t`, we make this indicator not applicable for those target functions that do not accept a parameter of that type. Or to be more exact, the indicator is not applicable for (cannot be used from) the replacement functions that do not provide a parameter of this type to indicator function. This limitation holds even if this parameter is not really used in the current scenario.



Although it is acceptable for the indicator to use the variables in expression, which are not derived from the indicator's parameters like

```
expression.variable.name = prob50
expression.variable.value = random() % 2
```

it is not recommended, because there is a more efficient way to do this. The thing is that, the variables of this kind are evaluated every time the indicator function is called, no matter if this variable is used in the expression or not. This evaluation may take relatively long time in some cases. There is another type of variables which is applicable in such cases - <i>runtime variables</i>. Declaration of such variables has the following format:

```
expression.rvariable.name = prob50
expression.rvariable.code =>>
    return random() % 2;
<<
```

`expression.rvariable.code` parameter provides the code of the function which will be used whenever value of variable is **really** needed. The costs of such optimisation are a function call used instead of the inlined code when the value of the variable is needed, and inability to use local variables of the indicator function (and parameters of this function) to prepared the value of the runtime variable.



To simplify writing expressions and to make them more readable, named constants can be declared and then used there. There are two ways to do this:

```
expression.constant.name = constant_100
expression.constant.value = 100
```

This makes the constant with name `constant_100` and value 100 available for usage in the expressions.

```
expression.constant.cname = GFP_ATOMIC
```

This makes the constant with name `GFP_ATOMIC` which evaluates to GFP\_ATOMIC available for usage in the expressions. Expression `flags == GFP_ATOMIC` is clearer and easier to read than `flags == 32`, isn't it?



Additionally, the following parameter can be defined:

<ul><li><b>global</b>

code (may be multiline) that will be inserted at the global scope and its definitions will be visible everywhere in the source file of the indicator. Usually, this code contains <code>#include</code> directives for the header files containing types definitions of parameters of the indicator and declarations of the functions used to obtain the values of the parameters.</li>
</ul>
Example of indicator for `kstrdup` function:

```
# This module implements indicator for kmalloc-like functions.

module.author = Andrey Tsyvarev
module.license = GPL

global =>>
#include <linux/gfp.h>      /* gfp_flags constants */
#include <linux/types.h>    /* size_t */
<<

indicator.name = kmalloc

indicator.parameter.type = size_t
indicator.parameter.name = size

indicator.parameter.type = gfp_t
indicator.parameter.name = flags

expression.constant.c_name = GFP_NOWAIT
expression.constant.c_name = GFP_KERNEL
expression.constant.c_name = GFP_USER
expression.constant.c_name = GFP_ATOMIC

expression.variable.pname = size
expression.variable.pname = flags
```




After writing `payload.data` file, you can change the value of module\_name variable in the `makefile` and `Kbuild` according to the one you use as value of "module.name" parameter.



The last step is to run <b><code>make</code></b> utility. This will invoke the code generator tool (<b><code>kedr_gen</code></b>) to create the sources for your payload module, then the module will be built.


## Implementing Custom Types of Analysis ##


KEDR framework also allows to implement custom analysis tools that process the information about the function calls made by the target module.



This section shows how to create a custom analysis system on top of KEDR. The system we are going to use as an example is rather simple: it maintains a set of counters accessible from user space that provide some information about the actions of the target module.



This analysis system will use neither call monitoring nor fault simulation facilities of KEDR. It will only rely on KEDR core and on the [API](kedr_manual_reference#API_for_Payload_Modules.md) it provides. Other types of analysis could be implemented in a way similar to this example.



In general, a custom analysis system based on KEDR can be created in the following steps.


<ol><li>
Determine which information about the actions of the target module should be processed by your analysis system. Decide whether it is enough to process (and may be alter to some extent) the function calls to collect this information. If so, KEDR could be of help here.<br>
</li>
<li>
Determine the calls to which functions your system needs to intercept to collect the necessary data or alter the behaviour of the target module in a required way. Note that it is only ordinary functions that count here rather than macros or inlines.<br>
</li>
<li>
Prepare a data file describing the signatures of the target functions of interest. This file is needed for KEDR to generate <a href='kedr_manual_glossary#Trampoline.md'>trampoline</a> functions. A <a href='kedr_manual_reference#A_Stub_of_the_Data_File_Describing_the_Targets.md'>skeleton of such data file</a> described in this manual and the examples we provide with KEDR can be helpful here.<br>
</li>
<li>
Generate the auxiliary source file containing the trampolines from the data file written at the previous step. This can be done using KEDR code generator as described in <a href='kedr_manual_reference#Generating_the_Source_Code_of_the_Trampolines.md'>"Generating the Source Code of the Trampolines"</a>.<br>
</li>
<li>
Prepare the source code of the payload module for KEDR that will process the intercepted functions. The examples we provide with KEDR as well as the <a href='kedr_manual_reference#A_Stub_of_a_Payload_Module.md'>skeleton of a payload module</a> described in this manual can be used as a starting point here.<br>
</li>
<li>
Build the payload module from the source files created at the previous steps. This is done in almost the same way as for any other kernel module.<br>
</li>
</ol>

Once the above steps are completed, KEDR utilities can be used to load your payload module along with the KEDR core. You can now load the target module and your system will start analyzing it.


<blockquote><font><b><u>Note</u></b></font>

The source code of the analysis system developed in this example is available in<br>
<br>
<code>&lt;kedr_install_dir&gt;/share/kedr/examples/counters/</code>.<br>
<br>
</blockquote>

### Choosing the Counters and the Functions to Process ###


Suppose the following counters are going to be supported by our analysis system:


<ul><li>
total number of memory allocation attempts;<br>
</li>
<li>
number of memory allocation attempts that have failed;<br>
</li>
<li>
size of the largest memory block requested to be allocated;<br>
</li>
<li>
total number of mutex lock operations;<br>
</li>
<li>
mutex balance, i.e. the difference between the total numbers of lock and unlock operations.<br>
</li>
</ul>

To make the counters accessible from the user space, we can, for example, provide a file in `kedr_counters_example` directory in debugfs for each one of them.



Once we have decided which data concerning a target kernel module our system will be collecting and processing, we need to determine which function calls made by the module the system should intercept.



Consider the first three counters. All of them are related to memory allocation. To collect necessary data when the target module operates, we can  use call interception facilities provided by KEDR. When the target module calls some function that allocates memory, the corresponding function provided by our analysis system will also be called and update counters.



There is a number of memory allocation functions available for kernel modules. Assume for simplicity that we choose to process only the calls to the following ones:


<ul><li>
<code>void * __kmalloc(size_t size, gfp_t flags)</code>
</li>
<li>
<code>void * krealloc(const void *p, size_t size, gfp_t flags)</code>
</li>
<li>
<code>void * kmem_cache_alloc(struct kmem_cache *mc, gfp_t flags)</code>
</li>
</ul>
<blockquote><font><b><u>Note</u></b></font>

It should not be very hard to extend this example to support other functions that allocate memory like <code>vmalloc()</code>, <code>kstrdup()</code>, etc.<br>
<br>
</blockquote>


To collect data necessary to provide the remaining two counters, our system needs to process the calls to the operations with mutexes:


<ul><li>
<code>void mutex_lock(struct mutex *lock)</code>
</li>
<li>
<code>int mutex_lock_interruptible(struct mutex *lock)</code>
</li>
<li>
<code>int mutex_lock_killable(struct mutex *lock)</code>
</li>
<li>
<code>int mutex_trylock(struct mutex *lock)</code>
</li>
<li>
<code>void mutex_unlock(struct mutex *lock)</code>
</li>
</ul>
<blockquote><font><b><u>Note</u></b></font>

Note that the functions may be different for different variants and versions of the Linux kernel. There is no stable binary interface in the Linux kernel anyway. Please choose memory allocation operations and mutex-related functions appropriate for your kernel.<br>
<br>
</blockquote>

### Describing the Targets ###


At this step, we need to create a data file that describes the target functions of interest. It will be used to generate the source code of the trampoline functions for these target functions. You don't need to worry about how the trampolines should operate, they will be generated automatically at the next step. Just describe the target functions properly in the data file and the tools provided by KEDR will do the remaining tedious work. The file can be based on the skeleton given in ["A Stub of the Data File Describing the Targets"](kedr_manual_reference#A_Stub_of_the_Data_File_Describing_the_Targets.md).
Here are the portions of the file corresponding to the memory allocation functions we have chosen above:

```
header=>>
#include <linux/slab.h>
...
<<

[group]
    function.name = __kmalloc
    returnType = void *
    
    arg.type = size_t
    arg.name = size
    
    arg.type = gfp_t
    arg.name = flags

[group]
    function.name = krealloc
    returnType = void *
    
    arg.type = void *
    arg.name = p
    
    arg.type = size_t
    arg.name = size
    
    arg.type = gfp_t
    arg.name = flags

[group]
    function.name = kmem_cache_alloc
    returnType = void *
    
    arg.type = struct kmem_cache *
    arg.name = mc
    
    arg.type = gfp_t
    arg.name = flags

...
```






### Generating the Trampolines ###


Assuming that the data file you have written in the previous section is named  `functions_support.data`, generation of the source file containing the definition of trampoline functions cay be performed as follows:


```
<kedr_install_dir>/lib/kedr/kedr_gen \
    <kedr_install_dir>/share/kedr/templates/function_support.c \
    functions_support.data > functions_support.c
```


This will generate file `functions_support.c` with the appropriate definitions.


### Creating the Payload Module ###


To implement our analysis system, we need to create [a payload module](kedr_manual_glossary#Payload_module.md) for KEDR. As a starting point, we can use, for example, the skeleton of a module given in ["A Stub of a Payload Module"](kedr_manual_reference#A_Stub_of_a_Payload_Module.md). Because our payload does not need to change the behaviour of the target functions, we use [post handlers](kedr_manual_glossary#Post_handler.md) for all of these except `mutex_unlock`. For `mutex_unlock`, we use [pre handler](kedr_manual_glossary#Pre_handler.md) (see ["trace.happensBefore Parameter for Call Monitoring"](kedr_manual_extend#trace.happensBefore_Parameter_for_Call_Monitoring.md)).



The instance of `struct kedr_payload` could be filled as follows (this structure should be used when registering and unregistering the payload module with KEDR core):


```
/* Post handler pairs */
statuc kedr_post_pair post_pairs[] = {
    { (void *)&__kmalloc, (void *)&post_kmalloc},
    { (void *)&krealloc, (void *)&post_krealloc},
    { (void *)&kmem_cahche_alloc, (void *)&post_kmem_cache_alloc},
    { (void *)&mutex_lock_interruptible, (void *)&post_mutex_lock_interruptible},
    { (void *)&mutex_lock_killable, (void *)&post_mutex_lock_killable},
    { (void *)&mutex_trylock, (void *)&post_mutex_trylock},
    { NULL,}
};

/* Pre handler pairs */
statuc kedr_pre_pair pre_pairs[] = {
    { (void *)&mutex_unlock, (void *)&pre_mutex_unlock},
    { NULL,}
};



static struct kedr_payload counters_payload = {
    .mod                    = THIS_MODULE,
    .post_pairs             = post_pairs,
    .pre_pairs              = pre_pairs,
    .replace_pairs          = NULL,
    .target_load_callback   = NULL,
    .target_unload_callback = NULL
};
```


The initial value of each counter is 0. Post handlers actually update the counters. They do this with special locks held to avoid some of the concurrency issues. For example, the post handler for `__kmalloc()` looks like this:


```
static void
post___kmalloc(size_t size, gfp_t flags, void *ret_val,
    struct kedr_function_call_info *call_info)
{
    unsigned long irq_flags;
    
    spin_lock_irqsave(&spinlock_alloc_total, irq_flags);
    ++cnt_alloc_total;
    spin_unlock_irqrestore(&spinlock_alloc_total, irq_flags);
    
    spin_lock_irqsave(&spinlock_alloc_failed, irq_flags);
    if (ret_val == NULL) ++cnt_alloc_failed;
    spin_unlock_irqrestore(&spinlock_alloc_failed, irq_flags);
    
    spin_lock_irqsave(&spinlock_alloc_max_size, irq_flags);
    if (size > cnt_alloc_max_size) cnt_alloc_max_size = size;
    spin_unlock_irqrestore(&spinlock_alloc_max_size, irq_flags);
}
```


This handler updates the variables corresponding to the relevant counters, `cnt_alloc_total`, `cnt_alloc_failed` and `cnt_alloc_max_size`, according to arguments of target functions and its return value.



The technical details concerning the creation of files for the counters in debugfs, are not described here. If you are interested in these details, see the source code of "Counters" example.


### Building the Payload Module ###


The payload module that we have prepared can be built much in the same way as any other kernel module. Still, there is a couple of things to take into account.



First, the module uses header files provided by KEDR, so the top include directory of KEDR should be specified in `-I` compiler option. The directory is usually `<kedr_install_dir>/include/`.



Second, each payload module uses functions exported by KEDR core and therefore needs the appropriate .symvers file. Before building the module, you should copy `kedr_base.symvers` file provided by KEDR to the directory of the payload module and rename it to `Module.symvers`. `kedr_base.symvers` is usually located in ``/lib/modules/`uname -r`/symvers/`` or in ``<kedr_install_dir>/lib/modules/`uname -r`/symvers/`` in case of a non-global installation of KEDR.



You can look at `Kbuild` and `makefile` files to see how the payload is built in "Counters" example.


### Using the Payload Module ###


Now that the payload module for our analysis system is built, we can use it to see how the values of the counters change as the target module operates. You can choose any kernel module as a target if you know how to properly load it and to make it operate.



Our analysis system makes the counters available via the files in debugfs. So if debugfs is not mounted (usually its directory is `/sys/kernel/debug/`), mount it first to a directory of your choice. For example,


```
mount debugfs -t debugfs some_dir/debugfs
```


Now it is time to load KEDR core and `kedr_counters.ko` payload module that we have built before. The easiest way is
probably to create a configuration file, say, `my.conf`, with the following contents:


```
payload path_to_example_directory/kedr_counters.ko
```


and use <b><code>kedr start</code></b> with that file:


```
kedr start <name_of_target_module> my.conf
```


See ["Controlling KEDR"](kedr_manual_using_kedr#Controlling_KEDR.md) for a detailed information about the configuration files, <b><code>kedr start</code></b>, etc.



Load target module and do something with it. While it is working (and also after it is unloaded), you can check how the counters are shown in the
files in `kedr_counters_example` subdirectory in debugfs.


```
tester@lab-x86:> cd /sys/kernel/debug/kedr_counters_example/
tester@lab-x86:> ls
alloc_failed  alloc_max_size  alloc_total  mutex_balance  mutex_locks

tester@lab-x86:> cat alloc_max_size 
Maximum size of a memory chunk requested: 48
```

<blockquote><font><b><u>Note</u></b></font>

Note that if you unload the target module and then load it again while the analysis system (KEDR core modules and <code>kedr_counters.ko</code> payload module) is loaded, the counters will not be reset. If you need them to reset in such situations, you can implement it yourself using target load/unload callbacks (see <a href='kedr_manual_reference#struct_kedr_payload.md'>"struct kedr_payload"</a>).<br>
<br>
</blockquote>