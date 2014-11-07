mscript
=======

A CLI  to MATLAB engine, that treats stdin nicely  (similar to Rscript). The problem is that MATLAB and mathworks is not willing to build a nice command line interface.

Usage
-----

Code comes from <STDIN>

```bash
cat test.m | ./mscript.sh
```

or from the file specified aas the first argument

```bash
./mscript.sh test.m
```

In the second case stdin is not quined by matlab engine. 

Note: this works only if you have a fully functional local install.

Disclamer
---------
All of this code is licensed under MIT.

MATLAB is ragistered trademark of Mathworks.

