### QPULib Language Manual

1. [Types](#types)

2. [Arithmetic operations](#arithmetic-operations)

3. [Type conversions](#type-conversions)

##### Types

Valid QPU types, denoted &tau;, are defined by the following grammar.

  &tau;               | Representation
  ------------------- | -------------------------------------------
  `Int`               | 16 x 32-bit integers
  `Float`             | 16 x 32-bit floats
  <tt>Ptr<&tau;></tt> | 16 x 32-bit pointers to values of type <tt>&tau;</tt>

##### Arithmetic operations

Binary operations on `Int` include

`+`, `-`, `*`, `min`, `max`, `<<`, `>>`, `&`, `|`, `^` and `~`

but beware: `*` considers only the first 24 bits of its operands!

Binary operations on `Float` include

`+`, `-`, `*`, `min`, `max`

with no restrictions on the arguments to `*`.

The only supported operations on <tt>Ptr&lt;&tau;&gt;</tt> are:

  * `p + i` where `p` has type <tt>Ptr&lt;&tau;&gt;</tt> and `i` has
     type `Int`; and

  * `*p` where `p` has type <tt>Ptr&lt;&tau;&gt;</tt> and the result
    is of type <tt>&tau;</tt>.


##### Type conversions

The expression `toFloat(x)` converts `x` from an `Int` to a `Float`.

Similarly, `toInt(x)` converts `x` from a `Float` to an `Int`.

When invoking a kernel, QPULib performs the following conversions
from CPU types to QPU types automatically.


  CPU Type              | QPU Type
  --------------------- | ----------------
  `int`                 | `Int`
  `float`               | `Float`
  `SharedArray<int>`    | `Ptr<Int>`
  `SharedArray<float>`  | `Ptr<Float>`
  `SharedArray<int*>`   | `Ptr<Ptr<Int>>`
  `SharedArray<float*>` | `Ptr<Ptr<Float>>`
