
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

- [ ] Document openGL issue on RPi 3
- [ ] Explanation code
  - [ ] 16-item vectors
  - [ ] Code Generation, not direct execution
- [ ] Drill-down of the bare essentials for understanding `VideoCore`
- [ ] 'Getting Started' to front page
- [ ] Examples to separate page under Docs


## Unit Tests

- [ ] Convert `AutoStart` to unit test
- [ ] Test same output for the various versions of `Rot3D`


### Unit testing for Debug
- [ ] All combinations of options for `Debug::enable()`
- [ ] Sequence: Open to file, open to other file
- [ ] Sequence: to file, off, to same file, off, to different file
- [ ] Sequence: stdout, to file, stdout


## Debug class

- [ ] Consider renaming to `Log` and treat as such
- [ ] Append (with header) instead of writing over existing logs


## Other

- [ ] Add TODO to project
- [ ] Add check in emulator for too many `gather()` calls
- [ ] `Rot3D` make various versions selectable on command line
- [ ] Prevent compile on non-RPi

Additionally: should it be compilable at all?
I.e. Following happens on compilation with `make QPU=1` on 64-bit Intel Linux:

```
SharedArray.h:121:14: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
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

So the question is how far QPULib programs are off hand-written QPU assembly, and what we can do to get closer.


### Set up Guidelines for the Project

This blog contains great tips for setting up open source projects: 

[The Bitter Guide to Open Source](https://medium.com/@ken_wheeler/a-bitter-guide-to-open-source-a8e3b6a3c1c4) by Ken Wheeler.

One day, I would like to convert the points mentioned here to a checklist and implement these for `QPULib`.
