![Greenbone Logo](https://www.greenbone.net/wp-content/uploads/gb_new-logo_horizontal_rgb_small.png)

# Greenbone Library for PostgreSQL utility functions <!-- omit in toc -->

This library contains PostgreSQL utility functions. For example to compute host
and ical information within SQL statements.

- [Build and Installation](#build-and-installation)
  - [Prerequisites](#prerequisites)
  - [Configure and Build](#configure-and-build)
  - [Use the extension](#use-the-extension)
  - [Test the extension](#test-the-extension)
    - [Setup for tests](#setup-for-tests)
    - [Integration](#integration)
    - [Running the tests](#running-the-tests)
  - [Support](#support)
  - [Maintainer](#maintainer)
  - [Contributing](#contributing)
  - [License](#license)

# Build and Installation

## Prerequisites

- GCC
- cmake >= 3.5
- pkg-config
- libical >= 1.0.0
- glib >= 2.42
- PostgreSQL dev >= 9.6
- libgvm-base >= 20.8

Install these packages using (on Debian GNU/Linux bookworm 12):

```sh
apt-get install gcc cmake pkg-config libical-dev libglib2.0-dev postgresql-server-dev-15
```

and build the gvm-libs as described in the [README](https://github.com/greenbone/gvm-libs).

## Configure and Build

This extension can be configured, built and installed with the following commands:

```sh
cmake .
make && make install
```

## Use the extension

To use the extension in a database create the extension using

```sh
CREATE EXTENSION "pg-gvm";
```

## Test the extension

The tests are based on pgTAP, a unit test tool for PostgreSQL Databases.

### Setup for tests

Install pgTAP cloning the [repository](https://github.com/theory/pgtap.git)

and follow the instructions in the [setup documentation](https://pgtap.org/documentation.html#installation)

### Integration

To use pgTAP in a database use

```sh
CREATE EXTENSION IF NOT EXISTS pgtap;
```

as postgres user.
To check if the extension exists use

```sh
\dx
```

### Running the tests

The tests are located in the ```tests``` folder of this repository.

As postgres user run (replace MY_DATABASE with the real name of the database)

```sh
pg_prove -d MY_DATABASE tests/*.sql
```

## Support

For any question on the usage of `pg-vgm` please use the [Greenbone Community
Forum](https://forum.greenbone.net/). If you found a problem with the
software, please [create an issue](https://github.com/greenbone/pg-gvm/issues) on
GitHub. If you are a Greenbone customer you may alternatively or additionally
forward your issue to the Greenbone Support Portal.

## Maintainer

This project is maintained by [Greenbone AG](https://www.greenbone.net/).

## Contributing

Your contributions are highly appreciated. Please [create a pull
request](https://github.com/greenbone/pg-gvm/pulls) on GitHub. Bigger changes need
to be discussed with the development team via the [issues section at
github](https://github.com/greenbone/pg-gvm/issues) first.

## License

Copyright (C) 2025 [Greenbone AG](https://www.greenbone.net/)

Licensed under the [GNU General Public License v3.0 or later](LICENSE).
