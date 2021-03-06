[group]
function.name = mutex_trylock
trigger.code =>>
	struct mutex m;
	mutex_init(&m);
	if(mutex_trylock(&m))
		mutex_unlock(&m);
	mutex_destroy(&m);
<<
