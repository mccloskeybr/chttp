# chttp

Basic HTTP server implemented in C, cus why not.

Features:
- HTTP requests, responses abstracted to a convenient parsing struct.
- Multithreaded, allows for a configurable number of requests to be processed at once.

TODO:
- Properly propagate errors encountered during processing.
- Implement some form of server side HTTP rendering.
