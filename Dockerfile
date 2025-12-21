# Base image
FROM ubuntu:22.04

# Avoid interactive apt prompts
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    zlib1g \
    zlib1g-dev \
    libssl-dev \ 
    bash \
    && rm -rf /var/lib/apt/lists/*


# Working directory inside container
WORKDIR /workspace

# Start an interactive shell by default
CMD ["/bin/bash"]
