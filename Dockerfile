FROM ctftools/valgrind:2025-12-22

ENV DEBIAN_FRONTEND=noninteractive

# Install ONLY what your project needs
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    zlib1g-dev \
    libssl-dev \
    bash \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
CMD ["/bin/bash"]
