# This is largely copied from the official buildroot Dockerfile (check the file
# in "<buildroot>/support/docker")

FROM debian:bullseye-20210902

ENV DEBIAN_FRONTEND noninteractive

COPY apt-sources.list /etc/apt/sources.list

RUN 	dpkg --add-architecture i386 \
	&& \
	apt-get update -y \
	&& \
	apt-get install -y --no-install-recommends \
		bc \
		build-essential \
		bzr \
		ca-certificates \
		cmake \
		cpio \
		cvs \
		file \
		g++-multilib \
		git \
		libc6:i386 \
		libncurses5-dev \
		locales \
		mercurial \
		openssh-server \
		python3 \
		python3-flake8 \
		python3-nose2 \
		python3-pexpect \
		python3-pytest \
		qemu-system-arm \
		qemu-system-x86 \
		rsync \
		shellcheck \
		subversion \
		unzip \
		wget \
	&& \
	apt-get -y autoremove \
	&& \
	apt-get -y clean \
	&& \
	sed -i 's/# \(en_US.UTF-8\)/\1/' /etc/locale.gen \
	&& \
	/usr/sbin/locale-gen \
	&& \
	useradd -m builder

USER builder
WORKDIR /home/builder
ENV HOME /home/builder
ENV LC_ALL en_US.UTF-8
