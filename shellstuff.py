#encoding=utf-8

import subprocess
import re
import os
re_get_mime = re.compile('^\\s*Content-Type:\\s*(.*?)\\s*$', re.MULTILINE)

def call(program, catch_stdout=False, catch_stderr=False, stdin=None):
	print program, catch_stdout, catch_stderr
	proc = subprocess.Popen(program, 
		stdout=subprocess.PIPE if catch_stdout else None,
		stderr=subprocess.PIPE if catch_stderr else None)
	stdout, stderr = proc.communicate(stdin)
	print "Returncode: %s" % proc.returncode
	return proc.returncode, stdout, stderr

def start(program, silent=False):
	print program
	proc = subprocess.Popen(program,
		stdout=subprocess.DEVNULL if silent else None,
		stderr=subprocess.DEVNULL if silent else None)
	return proc.pid 
	
def curl(url, *args):
	print "curl: "+url
	ret, data, dummy = call(['curl', url]+list(args), catch_stdout=True)
	return None if ret != 0 else data
		
def wget(url, output, *args, **kw_args):
	if type(url) == unicode:
		url=url.encode('utf-8')
	if type(output) == unicode:
		output=output.encode('utf-8')
		
	print "wget: "+url + " → " + output
	args=list(args)
	get_mime = kw_args.pop('get_mime', False)
	if kw_args:
		raise Exception('Unknown keyword argument')
	
	if get_mime:
		args+= ['--server-response']

	ret, dummy, data = call(['wget', url, '-O', output]+args, catch_stderr = get_mime)
	
	if get_mime:
		return re_get_mime.findall(data)[0]
		
	return ret
	
def cp(src, dst, *args):
	if type(src) == unicode:
		src=src.encode('utf-8')
	if type(dst) == unicode:
		dst=dst.encode('utf-8')
	print "cp: "+src + " → " + dst
	return subprocess.call(['cp', src, dst]+list(args))
	
def clean_dir(path):
	call(['rm', '-r', path])
	if call(['mkdir', '-p', path])[0] != 0:					#Something went wrong
		raise Exception("Could not clean dir %s"%path) 
		
		

