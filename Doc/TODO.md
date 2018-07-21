
# TODO

Feel free to add points here.

- Checked items are implemented but not merged yet. the PR-s are pending.
- An item that is completed and merged to `development` is removed from the list.


## Makefile

- [X] Enable debug-build, for debugging. Currently, an indication is given in the Makefile how to do this.


## Documentation

- [ ] Document openGL issue on RPi 3
- [ ] Explanation code
  - [ ] 16-item vectors
  - [ ] Code Generation, not direct execution
- [ ] Drill-down of the bare essentials for understanding `VideoCore`
- [ ] Examples to separate page under Docs
- [ ] Mailbox functions link to reference and explanation two size fields
- [ ] QPUs always round *downward* (in Issue)
- [ ] Use of `BoolExpr` etc as lambda


## Unit Tests

- [ ] Add test on expected source and target output for pretty print in `compileKernel`. E.g. for `Rot3D`, `Tri` and `HeatMap`.
- [ ] Add tests to compare QPU and Emulation mode output (when build can be done with both)
- [ ] Language
  - [ ] Test missing `End` on blocks
  - [ ] Test missing `Else` without `If`


## Investigate

- [ ] Is the gather limit 8 or 4? This depends on threading being enabled, check code for this.
- [ ] Improve heap implementation and usage. the issue is that heap memory can not be reclaimed. Suggestions:
  - Allocate `astHeap` for each kernel. Perhaps also do this for other heaps
  - Increase heap size dynamically when needed
  - Use `new/delete` instead. This would defeat the purpose of the heaps, which is to contain memory usage
  - Add freeing of memory to heap definitions. This will increase the complexity of the heap code hugely


## Library Code

- [ ] Add check in emulator for too many `gather()` calls
- [x] Determine num QPUs from hardware
- [x] Add method to determine RPi hardware revision number via mailbox
- [IP] Add code for using the `Special Functions Unit (SFU)`, operations: `SQRT`, `RECIPSQRT`, `LOG`, `EXP`
- [ ] Add performance counters to interface of `RegisterMap`
- [ ] Add Logging class
- [ ] Add method for build/platform info, for display on startup of an application
- [ ] Make QPU execution timeout a runtime setting


## Other

- [ ] `Rot3D` make various versions selectable on command line
- [ ] enable `-Wall` on compilation and deal with all the fallout
- [ ] Scan current docs for typos, good language
- [ ] enable build for QPU and Emulation mode together
- [x] `Rot3DLib` handler for command line parameters


## Long Term

- [ ] Add optional doc generation with `doxygen`. This is only useful if there are a sufficient number of header comments.
- [ ] Viewer for graphic output (e.g. `Mandelbrot`). Should be really simple
- [ ] Scheduling of kernels - see VideoCore `fft` project.


-----

## Principles

- QPU-mode should not compile on non-RPi platforms


## Stuff to Consider

### Measure performance in various ways

E.g. compare between:

  - different iterations of a program
  - number of QPU's used
  - RPi versions
  
  
### The QPULib compiler doesn't do much in the way of optimisation.

So the question is how far QPULib programs are off hand-written QPU assembly, and what we can do to get closer.


### Set up Guidelines for the Project

This blog contains great tips for setting up open source projects: 

- [The Bitter Guide to Open Source](https://medium.com/@ken_wheeler/a-bitter-guide-to-open-source-a8e3b6a3c1c4) by Ken Wheeler.

One day, I would like to convert the points mentioned here to a checklist and implement these for `QPULib`.
