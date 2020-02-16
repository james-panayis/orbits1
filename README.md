## Introduction

Simple gravitational orbit visualizer  
  
It begins with two bodies  
Every few seconds (within the first minute or so) a grid of small bodies are spawned  
Tap to spawn a grid of tiny bodies  
  
For demo please see: https://jamespanayis.com/orbits1

## Build instructions

The C++ is compiled to WebAssembly and requires a working **emscripten** install  

run:

	make


## Install instructions

Run a local webserver and surf to it eg:

	python -m SimpleHTTPServer 8080

Browse to http://localhost:8080/build/orbits1.html

OR:

Copy files to webserver root:

	PREFIX=/path/to/web/root make install
	
Browse to your webserver.

