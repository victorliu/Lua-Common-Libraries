`complex` module
================

## Types
-----

### `complex`

Represents a complex number with real and imaginary parts.

#### Methods

None.

#### Metamethods

The following operations are defined:

1. `__tostring`
2. `__add` (add real/complex to real/complex)
3. `__sub` (subtract real/complex from real/complex)
4. `__unm` (negation)
5. `__mul` (multiply real/complex with real/complex)
6. `__div` (divide real/complex by real/complex)
7. `__pow` (exponentiate real/complex by real/complex)
8. `__len` (norm)
9. `__eq` (equality comparison)

#### Properties

##### `real`

Gets or sets the real part of the number.

Example:

    z = complex(1,2)
	z.real = 3
	print(3 == z.real)

- - -

##### `imag`

Gets or sets the imaginary part of the number.

Example:

    z = complex(1,2)
	z.imag = 3
	print(3 == z.imag)

- - -

##### `norm`

Gets or sets the norm (magnitude) of the number.

Example:

    z = complex(1,2)
	z.abs = 1
	print((math.sqrt(0.2) - z.real) < 1e-10)

- - -

##### `normsq`

Gets the squared norm of the number.

Example:

    z = complex(1,2)
	print(5 == z.normsq)

- - -

##### `norm1`

Gets the 1-norm of the number (sum of absolute values of real and imaginary parts).

Example:

    z = complex(1,2)
	print(3 == z.norm1)

- - -

##### `conj`

Returns the conjugate of the number

Example:

    z = complex(1,2)
	print(complex(1,-2) == z.conj)

- - -

##### `arg`

Returns the argument of the number in the range of [-pi, pi].

Example:

    z = complex(1,2)
	print(math.atan(2,1) - z.arg < 1e-10)

- - -

## Functions

### `complex.expi2pi(x)`

Returns `exp(2*pi*i*x)`, equivalent to `complex(math.cos(2*pi*x), math.sin(2*pi*x))`, but more accurate for common values of `x` like 0.5.

