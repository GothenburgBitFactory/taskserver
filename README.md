# Taskserver

Thank you for taking a look at Taskserver!

Taskserver is a daemon or service that will allow you to share tasks among
different client applications, primarily Taskwarrior.

Taskserver is compatible with Taskwarrior version 2.4.x and later, but works
best with the latest Taskwarrior.

## Setup

Taskserver setup is complex. Be very careful when following instructions.
Here is the only supported Taskserver setup guide. Ignore all others.

[taskserver-setup.pdf](https://github.com/GothenburgBitFactory/guides/blob/master/taskserver-setup/taskserver-setup.pdf)

For troubleshooting, here is the only supported Taskserver troubleshooting guide. Ignore all others.

[taskserver-troubleshooting.pdf](https://github.com/GothenburgBitFactory/guides/blob/master/taskserver-troubleshooting/taskserver-troubleshooting.pdf)

Almost every configuration problem is caused by not following the setup guide above carefully enough,
followed by not following the troubleshooting guide carefully enough.
If you cut corners or skip steps, it will not work.

## Documentation

There is extensive online documentation. You'll find all the details at
[https://taskwarrior.org/docs/#taskd](https://taskwarrior.org/docs/#taskd)

At the site you'll find online documentation, downloads, news and more. Additionally there
are three man pages installed:

* taskd(1)
* taskdctl(1)
* taskdrc(5)

## Support

For support options, take a look at [taskwarrior.org/support](http://taskwarrior.org/support)

Please use pull requests, or alternately send your code patches to
[support@gothenburgbitfactory.org](mailto:support@gothenburgbitfactory.org)

## Branching Model

We use the following branching model:

* `master` is the stable branch. Building from here is the same as building
  from the latest tarball, or installing a binary package. No development is
  done on the `master` branch.

* `1.2.0` is the current development branch. All work is done here, and upon
  release it will be merged to `master`. This development branch is not stable,
  may not even build or pass tests, and should be treated accordingly.
  Make backups.

## Installing

There are many binary packages available, but to install from source requires:

* git
* cmake
* make
* C++ compiler, currently gcc 4.7+ or clang 3.3+ for full C++11 support

Download the tarball, and expand it:

    $ curl -O https://taskwarrior.org/download/taskd-1.2.0.tar.gz
    $ tar xzf taskd-1.2.0.tar.gz
    $ cd taskd-1.2.0

Or clone this repository:

    $ git clone --recursive https://github.com/GothenburgBitFactory/taskserver.git
    $ cd taskserver

In case of errors with libshared (URL pointing to git.tasktools.org):

    $ sed -i 's/git.tasktools.org\/TM/github.com\/GothenburgBitFactory/' .git/config
    $ git submodule update

Then build:

    $ cmake -DCMAKE_BUILD_TYPE=release .
    ...
    $ make
    ...
    [$ make test]
    ...
    $ sudo make install

## Contributing

Your contributions are especially welcome.
Whether it comes in the form of code patches, ideas, discussion, bug reports, encouragement or criticism, your input is needed.

Visit [Github](https://github.com/GothenburgBitFactory/taskserver) and participate in the future of Taskserver.

## License

Taskserver is released under the MIT license.
For details check the [LICENSE](LICENSE) file.

