import os

pid = os.fork()

efficient_args=['main', 'default.cfg']

if pid:
	#parent
	print pid
else:
	#child
	os.execvp("main", ("main",) +  "default.cfg")
	#os.execl("../main", *efficient_args)