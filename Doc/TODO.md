
# TODO

Feel free to add points here, or check them off if done.


## Makefile

- [x] Raise the makefile one directory level and expand it
- [ ] Explanatory comment at top
- [ ] Static library for Lib-files
- [ ] Add include-file dependencies
- [ ] Get rid of DEBUG option -> make debug output dynamic
- [ ] Enable debug-build, for debugging
- [ ] Add unit testing, notably `CATCH`
- [ ] Add optional doc generation with `doxygen`

## Documentation

- [ ] document openGL issue on RPi 3
- [ ] explanation code
  - [ ] 16-item vectors
  - [ ] Code Generation, not direct run
- [ ] drill-down of the bare essentials for understanding VideCore
- [ ] 'Getting Started' to front page
- [ ] Examples to separate page under Docs

## Unit Tests
### Unit testing for Debug
- [ ] all combinations of options for `Debug::enable()`
- [ ] open to file, open to other file
- [ ] stdout, to file, stdout


## Debug class

- [ ] consider renaming to Log and treat as such
- [ ] append (with header) instead of writing over logs


## Other

- [ ] Add TODO to project
- [ ] Add check in emulator for too many `gather()` calls
- [ ] Prevent compile on non-RPi

Additionally: should it be compilable at all? I.e. following during compilation:

```
Lib/VideoCore/SharedArray.h:121:14: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
      gpu_base = (void*) mem_lock(mb, handle);
```

-----

## Stuff to Consider

### Measure performance in various ways
E.g.

  - different iterations of a program
  - number of QPU's used
  - compare between RPi version
  
### The QPULib compiler doesn't do much in the way of optimisation.

So another question is how far QPULib programs are off hand-written QPU assembly, and what we can do to get closer.

### Set up Guidelines for the Project

This blog contains great tips for setting up open source projects: 
(https://medium.com/@ken_wheeler/a-bitter-guide-to-open-source-a8e3b6a3c1c4)[The Bitter Guide to Open Source]] - Ken Wheeler.

One day, I would like to convert the points mentioned here to a checklist and implement these for `QPULib`.
