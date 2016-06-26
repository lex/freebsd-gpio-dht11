# DHT11 reading on FreeBSD

At least it works.

Uses [libgpio](https://bitbucket.org/rpaulo/libgpio/src) by Rui Paulo.

### Using
Change your pin, compile with:
```
$ clang main.c libgpio.c -o dht
$ ./dht
```
