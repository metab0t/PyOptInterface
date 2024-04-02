# Developer Guide

This section is intended for developers who want to contribute to the PyOptInterface library. It provides an overview of the codebase, the development process, and the guidelines for contributing to the project.

## Codebase Overview

The PyOptInterface library is a C++/Python mixed library. The core parts are implemented in C++ and exposed to Python via [nanobind](https://github.com/wjakob/nanobind). The build system is based on [scikit-build-core](https://github.com/scikit-build/scikit-build-core).

The codebase is organized as follows:

- `include/pyoptinterface`: The header files of the C++ library.
- `lib`: The source files of the C++ library.
- `src`: The Python interface.
- `tests`: The test cases.
- `thirdparty`: The third-party dependencies.

## Development Process

Supposing you want to contribute to PyOptInterface, you can follow these steps:

Firstly, fork the [PyOptInterface repository](https://github.com/metab0t/PyOptInterface) on GitHub and clone your forked repository to your local machine: `git clone https://github.com/<your-username>/PyOptInterface.git`.

Next, you should set up the development environment. The third-party optimizers must be configured following the instructions in [getting started](getting_started.md). In order to build PyOptInterface from source, you need the following dependencies:
- CMake
- A recent C++ compiler (We routinely test with GCC 10, latest MSVC and Apple Clang on the CI)
- Python 3.8 or later
- Python packages: `scikit-build-core`, can be installed by running `pip install scikit-build-core[pyproject]`

Then, you can build the project by running the following commands:
```bash
pip install --no-build-isolation -ve .
```

You will see a new `build` directory created in the project root. The Python package is installed in editable mode, so you can modify the source code and test the changes without reinstalling the package.

After making changes to the code, you should run the test cases to ensure that everything is working as expected. You can run the test cases by executing the following command (installing `pytest` is required):
```bash
pytest tests
```

The tests of PyOptInterface are still scarce, so you are encouraged to write new test cases for the new features you add.

Finally, you can submit a pull request to the [PyOptInterface repository](https://github.com/metab0t/PyOptInterface)

## Building Documentation

The documentation of PyOptInterface is built using [Sphinx](https://www.sphinx-doc.org/).

Firstly, you should install the dependencies for building the documentation:
```bash
pip install -r docs/requirements.txt
```

You can build the documentation by running the following commands:
```bash
cd docs
make html
```

The docs are built in the `docs/build/html` directory. You can open the `index.html` file in a web browser to view the documentation.

## Contributing Guidelines

When contributing to PyOptInterface, please follow these guidelines:

- Make sure that the code is well formatted and documented. The C++ code is formatted using [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the Python code is formatted using [black](https://black.readthedocs.io/en/stable/).
- For big changes like adding interface for a new optimizer, please open a thread in [**Discussion**](https://github.com/metab0t/PyOptInterface/discussions) to discuss the design before starting the implementation.
