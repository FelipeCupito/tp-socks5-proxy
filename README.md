# Proxy Socks5 and Wannapprove Protocol

## Authors
* Cupitó, Felipe
* De Luca, Juan Manuel
* Kim, Azul Candelaria
* Maggiony Duffy, Faustino

## Sections of this README
1. Source Code
1. Compiling
1. Execution
	1. Server
	1. Client
		1. Authentication
		1. GET Action
		1. PUT Action
		1. EDIT Action
		1. CONFIGBUFFSIZE Action
		1. CONFIGSTATUS Action
		1. DELETE Action
1. Cleaning

## 1. Source Code
In this repository, four main directories can be found, these are: [src](./src), where all the source code regarding the implementation of the proxy and parsers; [include](./include), where the .h files for the src directory are located; [management_protocol](./management_protocol), where the files regarding the Wannapprove Protocol are located, with their respective .h files; and [test](./test), where all the testing files are located. Additionally, the man files for the [Wannapprove Protocol](./wannapprove.8) and the [Proxy](./socks5d.8), together with the [Documentation of the Wannapprove Protocol](./Wannapprove_Protocol_v0.txt) and the [report of the project](./Grupo6_Informe.pdf) can be found in the root directory of this repository.

## 2. Compiling
Compiling the project is a rather simple process. In order to compile only the server, one must run:
```
$ make server
```
On the other hand, if one wanted to only compile the client, they should run:
```
$ make client
```
Finally, to compile everything, one must run:
```
$ make all
```

## 3. Execution
After successfully compiling the project, one can execute each part of if following the instruccions below.

### 3.i. Server
```
$ ./server
```
By default, the proxy listens to port 1080. For IPv4, it uses the 127.0.0.1 address and for IPv6, ::1.
For the proxy configuration, it listens to port 8080 and uses the same address as the proxy by default.

There are different methods allowed by the server to be run, these are:

#### 3.i.a Helper
In order to view all commands supported by the proxy, run:
```
$ ./server -h
```

#### 3.i.b Address
In order to change the address used by the proxy, run:
```
$ ./server -l <ADDRESS>
```

In order to modify the address used by the managemente service, run:
```
$ ./server -L <ADDRESS>
```

#### 3.i.c Ports
In order to change the port that listens to the incoming SOCKS connections, run:
```
$ ./server -p <PORT>
```

In order to modify the port that listens to the incoming Wannapprove connections, run:
```
$ ./server -P <PORT>
```

#### 3.i.d Request example
This is an example of how to use curl with the default settings:
```
$ curl -x socks5h://127.0.0.1:1080 www.google.com
```

For more information on how to use the server, execute in the terminal:
```
$ man ./socks5d.8
```

### 3.ii. Client
There are different methods allowed by the client to be run, these are:

#### 3.ii.a. Authentication
In order to correctly perform the actions allowed by this client, one must be properly authenticated as the admin, for which one must possess the admin password, which for this project is 'holacomoestas', but it can be changed. The correct way to authenticate with the client is:
```
$ ./client -a holacomoestas [ACTIONS]
```
Where [ACTIONS] refer to the actions that can be perfomed by the client. If no action is indicated, the client will only authenticate and perform no action.

#### 3.ii.b. GET Action
```
$ ./client -a password -g [OPTION]
```
Where [OPTION] can be one of the following:
* users (gets all users with theis passwords)
* passwords (gets all sniffed passwords)
* buffersize (gets the size of the buffer)
* authstatus (gets the authentication status)
* spoofstatus (gets the spoofing status)
* sentbytes (gets the amount of sent bytes)
* rcvbytes (gets the amount of received bytes)
* historic (gets the amount of historic connections)
* concurrent (gets the amount of concurrent connections)

#### 3.ii.c. PUT Action
This inserts a new user with username and password.
```
$ ./client -a password -i username:password
```
Where 'username' is de username and 'password' is the password for the new user.

#### 3.ii.d. EDIT Action
This edits either the username or password of an existing user.
```
$ ./client -a password -e username:[OPTION]:newfield
```
Where [OPTION] can be:
* 0 for editing the username of the user indicated by 'usuername', in which case 'newflield' will be the new username.
* 1 for editing the password of the user indicated by 'username', in which case 'newfield' will be the new password.

#### 3.ii.e. CONFIGBUFFSIZE Action
This allows the admin to configure the size of the buffer.
```
$ ./client -a password -b newbuffersize
```
Where 'newbuffersize' refers to the amount of bytes for the new buffer size.

#### 3.ii.f. CONFIGSTATUS Action
This allows the admin to configure the status of the authentication or the spoofing.
```
$ ./client -a password -t [OPTION]:[STATUS]
```
Where [OPTION] can be:
* auth, to modify the authentication status.
* spoof, to modify the spoofing status.

And where [STATUS] can be:
* on, to turn the option on.
* off, to turn the option off.

#### 3.ii.g. DELETE Action
This allows the admin to delete an existing user.
```
$ ./client -a password -d username
```
Where 'username' is the username of the user that is being deleted.


For more information on how to use the client for Wannapprove protocol, run:
```
$ man ./wannapprove.8
```

## 4. Cleaning
In order to delete all the binary and executable files generated in compilation, run:
```
$ make clean
```