FROM ubuntu:latest
#
# Install C, C++, python, misc tools and libs, GNU plot (headless), SQLite:
#
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y gcc g++ build-essential valgrind git vim micro dos2unix make cmake
RUN apt-get install -y zlib1g-dev libssl-dev bash 

# cleanup to reduce image size:
#
RUN apt-get clean && rm -rf /var/lib/apt/lists/*
#
# turn off history file creation:
#
RUN echo "export HISTFILE=/dev/null" >> /etc/profile
#
# add a user (with no pwd) so we don't run as root:
#
RUN useradd -m -g users user

# switch to the new user:
WORKDIR /workspace
