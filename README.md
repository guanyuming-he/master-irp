# What is this project
This project contains my 3 month Independent Research Project related files for
my Master's at Imperial College London in 2024--25.

The project was originally private and under the authorship of my Imperial
GitHub account (see the commit history). Agreed by my supervisors, I could
publish this repository. 

For publish's sake, I made a few modifications to this repo:
- The final updated slides for my thesis is here, not in that private repo.
- Only the public readme has this section.
- I removed all the GitHub actions from the private repo.


# What is this README About
Since all important background, methodology, results, and discussion can be
found in the main thesis (gh124-final-report.pdf), the README is ONLY concerned
with how to setup and build the project, with the requirements.

## Setup the $\LaTeX$ compilation
To compile my thesis (final report), project plan, presentation slides, one
must ensure she has a working environment where `latexmk` can run, and all the
necessary packages are installed.

I personally use the TeXLive distribution, but any other distribution with
sufficient packages should work as well.

## Setup the Python environment
For easy management of packages, I use Anaconda distribution for Python. 
To record all my installed packages, I exported them via the following
commands:
```
conda env export --from-history >./python-env/environment.yml
python3 -m pip freeze >./python-env/requirements.txt
```

One has to install all the packages listed in the two files to make my Python
scripts work on one's computer. 

### Detailed instructions
1. Install Anaconda
```
curl -LO https://repo.anaconda.com/archive/Anaconda3-2025.06-0-Linux-x86_64.sh | sh
```

2. Create a conda environment and install packages.
```
cd ./python-env
conda env create -f environment.yml
conda activate irp
pip install -r requirements.txt
```

3. Create a venv using system Python for C++ programs to link
First, make sure your system Python can have `python3 -m venv`. 
Then, execute
```
python3 -m venv ./venv
source ./venv/bin/activate
python3 -m pip install htmldate
```

Note that I use Python version 3.11, not the latest, to be compatible with
`ollama` and `open-webui`.


## Setup the environment to compile C++ programs
Setting up C++ programs is the hardest among all, because there's no concept as
a package in C++, and libraries are managed and installed in different ways.

However, there's a handy list of all the libraries that are needed, just inside
my `CMakeLists.txt`. I believe it describes where I expect there libraries to
be put in the system. To not mess things up, I link to the CPython libraries
from a venv, the one you setup in the previous section.

### Detailed instructions
0. Make sure you have a working C and a working C++ compiler. The C++ compiler
   must support must C++20 features. (I use `g++` 12).
1. Download and install CMake the way you like from https://cmake.org/download/
You should also have `GNU Make` available.
2. Install `libcurl`. On Debian or its derived GNU/Linux (e.g. Ubuntu, PureOS),
execute 
```
apt-get install libcurl4-gnutls-dev
```
or goto https://curl.se/download.html for other ways to install
3. Install `xapian`. On Debian or its derived GNU/Linux (e.g. Ubuntu, PureOS),
execute 
```
apt-get install libxapian-dev
```
or goto https://xapian.org/docs/install.html for other ways to install
4. Install `lexbor`. Once you have cmake installed, download the source and
   build `lexbor`. Finally install it. More specifically,
```
git clone https://github.com/lexbor/lexbor.git
cmake . -DLEXBOR_BUILD_TESTS=ON 
make
make test
```
After the tests have passed, then, as `root`, execute
```
cmake --install .
```
5. Install `Boost.unit_test_framework` and `Boost.url`.
On Debian or its derived GNU/Linux (e.g. Ubuntu, PureOS),
execute 
```
apt-get install libboost-test-dev libboost-url-dev
```
or goto https://www.boost.org/library/latest/test/ and 
https://www.boost.org/library/latest/url/ for other ways to install.
6. Install `pugixml`.
```
curl -LO https://github.com/zeux/pugixml/releases/download/v1.15/pugixml-1.15.tar.gz
tar -xzf ./pugixml-1.15.tar.gz
cd ./pugixml-1.15
mkdir ./bin
cmake -S . -B ./bin
cmake --build ./bin
```
Finally, execute as `root`
```
cmake --install ./bin
mkdir /usr/local/include/pugixml
cp /usr/local/include/pugixml*.hpp -t /usr/local/include/pugixml
```

### Test installation and compile
If everything goes well, then at the project root, 
activate venv
```
source ./venv/bin/activate
```

execute
```
make config-dbg
```
Should succeed in finding all libraries. If it fails in finding Python, make
sure you have development files (on Debian based systems, run `apt-get install
python3-dev`).
Then, execute
```
make build
```
If everything succeeds, then the build should succeed.

Finally, run 
```
make test
```
to test the C++ programs.

## Other requirements
One should also install `ollama` and the models one wants to use (by default,
it's those inside the default `config.json`, which is generated by the pipeline
for the first time; one can also look into `src/pipeline/config.py` to check the models).

More specifically, after one checks the models, one should run for each model
(remember to run `ollama serve` first).
```
ollama pull <model_name>
```

To run these modells smoothly in one's local computer, one should have
recommended at least 16GiB RAM and a GPU with at least 8GiB VRAM. It is
possible, indeed, to run such models without a dedicated GPU or less RAM, but
that would make the process much more slow.

One should have `cron` (which is by default installed on most GNU/Linux and
Unix-like OS), for the task schedulling feature to work.

## Initialize the system.
Before the system can be used, it must be initialized. 

1. One must setup an initial database for the search engine.
Run 
```
./bin/indexer ./db ./que false <how_many>
```
It is recommended to have at least 10,000 as `<how_many>`.
2.
Activate Anaconda Python3. You must exit previous Python venv first
```
deactivate
source ~/anaconda3/bin/activate irp
```
3. 
Configure the system initially
```
python3 ./src/pipeline/config_gui.py
```
4. Setup task schedulling
```
python3 ./src/pipeline/setup_schedules.py
```

It's done!

After the setup, outside of scheduled times, you can also run the pipeline
manually via (be sure to run `ollama serve` first!)
```
python3 ./src/pipeline/llm_pipeline.py
```

