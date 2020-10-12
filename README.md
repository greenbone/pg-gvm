![Greenbone Logo](https://www.greenbone.net/wp-content/uploads/gb_logo_resilience_horizontal.png)

# Greenbone Library for ical functions

This library contains functionality for ical object manipulation.

# Build and Installation

## Prerequisites

* GCC
* cmake >= 3.0
* pkg-config
* libical >= 1.0.0
* PostgreSQL dev >= 9.6

Install these packages using (on Debian GNU/Linux 'Buster' 10):

```sh
apt-get install gcc cmake pkg-config libical-dev postgresql-server-dev-11
```

## Configure and Build

This extension can be configured, built and installed with the following commands:

```sh
cmake .
make && make install
```
## Use the extension

To use the extension in a database create the extension using

```sh
CREATE EXTENSION pg_gvm;
```
