#### mwss_test

This is a test project to interface Stephan Held's MWSS algorithm for the Maximum Weight Stable Set problem into a C++ codebase.

##### How to build

[Exactcolors](https://github.com/heldstephan/exactcolors) is required to build this project. Let `${EC_HOME}` be the installation path of Exactcolors. This means that:

* `${EC_HOME}/include` contains all the `.h` headers of the library
* `${EC_HOME}/lib` contains `libsewell.a`

Once Exactcolors is installed, build the project as follows:

* `git clone git@github.com:alberto-santini/mwss_test.git`
* `cd mwss_test`
* `mkdir build && cd build`
* `cmake -DEXACTCOLORS_ROOT_DIR=${EC_HOME} -DCMAKE_BUILD_TYPE=Debug ..`
* `make`

##### License

This software is distributed under the GNU General Public License v3, as detailed in `LICENSE.txt`.