# indicator_internal.data - intermediate representation of the indicator data

module.author = <$module.author$>
module.license = <$module.license$>
indicator.name = <$indicator.name$>

<$indicatorParameters: join(\n)$>

<$if concat(global)$><$globalSection: join(\n)$><$endif$>

###############  Expresion  ####################

# Declarations for the expression

global =>>
#include <kedr/calculator/calculator.h>

#include <kedr/base/common.h> /* in_init */
#include <linux/random.h> /* random32() */

#include <kedr/util/stack_trace.h> /* kedr_save_stack_trace() */

/*
 * This is distance in the stack from returning address of
 * the indicator simulate function to the returning address of
 * the replacement function.
 * 
 * Value is very subtle, it depends on implementation
 * of the indicator module, fault_simulation module, payload module.
 * 
 * But it doesn't depend on kernel and kernel configuration
 * (this dependency is hidden in kedr_save_stack_trace()).
 */
#define REPLACEMENT_STACK_DEPTH 3
// Constants in the expression
<$expressionConstDeclarations$>

// Variables in the expression
static const char* var_names[]= {
    "times",//local variable of indicator state
    "caller_address",//address of the caller of the replacement function
<$expressionVarNames$>
};
// Runtime variables in the expression
static kedr_calc_int_t in_init_weak_var_compute(void)
{
    return kedr_target_module_in_init();
}

static kedr_calc_int_t rnd100_weak_var_compute(void)
{
    return random32() % 100;
}

static kedr_calc_int_t rnd10000_weak_var_compute(void)
{
    return random32() % 10000;
}

static const struct kedr_calc_weak_var weak_vars[] = {
    { .name = "in_init", .compute = in_init_weak_var_compute },
    { .name = "rnd100", .compute = rnd100_weak_var_compute },
    { .name = "rnd10000", .compute = rnd10000_weak_var_compute },
<$expressionRvarDeclarations$>
};


<<

indicator.state.name = calc
indicator.state.type = kedr_calc_t*

indicator.state.name = expression
indicator.state.type = char*

indicator.state.name = times
indicator.state.type = atomic_t

# Simulate for expression
indicator.simulate.name = expression
indicator.simulate.first =
indicator.simulate.code =>>
	int result;
    int *kcalc;
    unsigned long entries[REPLACEMENT_STACK_DEPTH];
    unsigned int nr_entries;
	kedr_calc_int_t vars[ARRAY_SIZE(var_names)];

    kedr_save_stack_trace(entries, ARRAY_SIZE(entries),
        &nr_entries);

    vars[0] = atomic_inc_return(&state(times));
    vars[1] = (nr_entries == ARRAY_SIZE(entries))
        ? (kedr_calc_int_t)entries[ARRAY_SIZE(entries) - 1]
        : -1;
    //debug
    //pr_info("Caller address is (%p)%pF", (void*)vars[1], (void*)vars[1]);
<$expressionVarsSet$>

    rcu_read_lock();
    kcalc = (int *)(state(calc));
    result = kedr_calc_evaluate((kedr_calc_t *)(rcu_dereference(kcalc)), vars);
    rcu_read_unlock();
	return result;
<<

#Initialize expresssion part of the indicator

indicator.init.name = expression
indicator.init.code =>>
	const char* expression = "0";//hardcoded
	// Initialize expression
    atomic_set(&state(times), 0);
    
    state(calc) = kedr_calc_parse(expression,
        <$expressionConstParams$>,
        ARRAY_SIZE(var_names), var_names,
        ARRAY_SIZE(weak_vars), weak_vars);
    if(state(calc) == NULL)
    {
        pr_err("Cannot parse string expression.");
        return -1;
    }
    state(expression) = kstrdup(expression , GFP_KERNEL);
    if(state(expression) == NULL)
    {
        pr_err("Cannot allocate memory for string expression.");
        return -ENOMEM;
    }
	return 0;
<<

#Destroy expresssion part of the indicator

indicator.destroy.name = expression
indicator.destroy.code =>>
    if(state(expression) != NULL)
        kfree(state(expression));
    if(state(calc) != NULL)
        kedr_calc_delete(state(calc));
<<

# Control file for the expression

indicator.file.name = expression
indicator.file.fs_name = expression
indicator.file.get =>>
	return kstrdup(state(expression), GFP_KERNEL);
<<
indicator.file.set =>>
	char *new_expression;
    kedr_calc_t *old_calc;
    kedr_calc_t *new_calc;
    
    new_calc = kedr_calc_parse(str,
        <$expressionConstParams$>,
        ARRAY_SIZE(var_names), var_names,
        ARRAY_SIZE(weak_vars), weak_vars);
    if(new_calc == NULL)
    {
        pr_err("Cannot parse expression");
        return -EINVAL;
    }

    new_expression = kstrdup(str, GFP_KERNEL);
    if(new_expression == NULL)
    {
        pr_err("Cannot allocate memory for string expression.");
        kedr_calc_delete(new_calc);
        return -ENOMEM;
    }
    
    old_calc = state(calc);
    {
        int *kcalc = (int *)new_calc;
        int **tmp = (int **)(&state(calc));
        rcu_assign_pointer(*tmp, kcalc);
    }
    
    synchronize_rcu();

    kfree(state(expression));
    state(expression) = new_expression;

    kedr_calc_delete(old_calc);
    
    return 0;
<<

################# PID ##################

indicator.state.name = pid
indicator.state.type = atomic_t

# Declarations for pid
global =>>
#include <linux/sched.h> /* task_pid */
<<

# Simulate for pid
indicator.simulate.name = pid
indicator.simulate.first = yes
indicator.simulate.code =>>
    struct task_struct* t, *t_prev;
	int may_simulate = 0;
	pid_t pid;
	smp_rmb(); //volatile semantic of 'pid' field
	pid =  (pid_t)atomic_read(&state(pid));
    if(pid == 0) return 0;
    
    //read list in rcu-protected manner(perhaps, rcu may sence)
    rcu_read_lock();
    for(t = current, t_prev = NULL; (t != NULL) && (t != t_prev); t_prev = t, t = rcu_dereference(t->parent))
    {
        if(task_tgid_vnr(t) == pid) 
        {
            may_simulate = 1;
			break;
        }
    }
    rcu_read_unlock();
	if(!may_simulate) simulate_never();
    return 0;
<<

# Initialize for pid
indicator.init.name = pid
indicator.init.code =>>
	atomic_set(&state(pid), 0);
	return 0;
<<

# Destroy for pid does not need.

# Control file for pid

indicator.file.name = pid
indicator.file.fs_name = pid
indicator.file.get =>>
	char *str;
    int str_len;
	pid_t pid = atomic_read(&state(pid));

    //write pid as 'long'
    str_len = snprintf(NULL, 0, "%ld", (long)pid);
    
    str = kmalloc(str_len + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("Cannot allocate string for pid");
        return NULL;
    }
    snprintf(str, str_len + 1, "%ld", (long)pid);
    return str;
<<
indicator.file.set =>>
    //read pid as long
    long pid_long;
    int result = strict_strtol(str, 10, &pid_long);
    if(!result)
        atomic_set(&state(pid), (pid_t)pid_long);
    return result ? -EINVAL : 0;
<<

# Control file for times

indicator.file.name = times
indicator.file.fs_name = times
indicator.file.get =>>
	char *str;
    int str_len;
	unsigned long times = (unsigned long)atomic_read(&state(times));

    str_len = snprintf(NULL, 0, "%lu", times);
    
    str = kmalloc(str_len + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("Cannot allocate string for times");
        return NULL;
    }
    snprintf(str, str_len + 1, "%lu", times);
    return str;
<<
indicator.file.set =>>
	atomic_set(&state(pid), 0);
	return 0;
<<