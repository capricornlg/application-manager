# Introduction
Application Manager is a daemon process to manage different type of sub-applications(process), each application can be a specific jobs, application namager will make sure all defined applications running on-time with defined behavior. provide REST APIs for integrate with outside app, provide command-line to start/stop and register new app easily.

The internal timer is multi-threaded with high-precision that can used to replace Linux cron-tab and supervisor.

![diagram.png](https://github.com/jinneec/application-manager/blob/master/doc/diagram.png?raw=true) 


Types of applications supported | Behavior
---|---
Long running application | Will always be restarted when exited
Short runing application | Will be launched periodly
Periodic long running application |Long running applicatin but will be restart periodic
Application avialable time range|Application can be only avialable in a specific time range daily (all app can have this behavior) with timezone definition


## Development tecnical
- [C++11](http://www.cplusplus.com/articles/cpp11/)
- [ACE-6.3.3](https://github.com/DOCGroup/ACE_TAO)
- [Microsoft cpprestsdk-2.10.1](https://github.com/Microsoft/cpprestsdk)
- [boost-1.58.0](https://github.com/boostorg/boost)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [google CCTZ (++TBD++)](https://github.com/google/cctz)

## Setup build environment on Ubuntu
```
apt-get install g++ git make zlib1g-dev libssl-dev cmake
apt-get install libboost-all-dev libcpprest-dev libjsoncpp-dev libace-dev  libcctz-dev
apt-get install ruby ruby-dev rubygems
gem install fpm
```

## REST APIs

Method | URI
---|---
GET | /app/$app-name
GET | /app-manager/applications
GET | /app-manager/config
PUT | /app/$app-name
POST| /app/$app-name?action=start
POST| /app/$app-name?action=stop
DELETE| /app/$app-name




## Show all sub command

```
$ appc
Commands:
  view        List application[s]
  config      Display configurations
  start       Start a application
  stop        Stop a application
  reg         Add a new application
  unreg       Remove an application

Run 'appc COMMAND --help' for more information on a command.

Usage:  appc [COMMAND] [ARG...] [flags]
```


## Display application[s]

```
$ appc view 
id user  active pid   return name        command_line
1  root  start  19574 0      abc         sleep 30
2  kfc   start  19575 0      def         ping www.google.com

$ appc view -n def
id user  active pid   return name        command_line
1  kfc   start  19575 0      def         ping www.google.com
```

## Display configurations

```
$ appc config
appc config
{
   "Applications" : [
      {
         "active" : 1,
         "command_line" : "sleep 30",
         "name" : "abc",
         "run_as" : "root",
         "working_dir" : "/opt"
      },
      {
         "active" : 1,
         "command_line" : "ping www.google.com",
         "name" : "def",
         "run_as" : "kfc",
         "working_dir" : "/opt"
      }
   ],
   "HostDescription" : "Host01",
   "RestListenPort" : 6060,
   "ScheduleIntervalSec" : 2
}
```

## Register a new application

```
$ appc reg
Register a new application::
  -n [ --name ] arg              application name
  -u [ --user ] arg              application process running user name
  -c [ --cmd ] arg               full command line with arguments
  -w [ --workdir ] arg (=/tmp)   working directory
  -a [ --active ] arg (=1)       application active status (start is true, stop
                                 is false)
  -t [ --time ] arg              start date time for short running app (e.g., 
                                 '2018-01-01 09:00:00')
  -s [ --daily_start ] arg       daily start time (e.g., '09:00:00')
  -d [ --daily_end ] arg         daily end time (e.g., '20:00:00')
  -e [ --env ] arg               environment variables (e.g., 
                                 env1=value1:env2=value2)
  -i [ --interval ] arg          start interval seconds for short running app
  -x [ --extraTime ] arg         extra timeout for short running app,the value 
                                 must less than interval  (default 0
  -k [ --keep_running ] arg (=0) monitor and keep running for short running app
                                 in start interval
  -f [ --force ]                 force without confirm.
  -h [ --help ]                  produce help message
  
$ appc reg -n def -u kfc -c 'ping www.google.com' -w /opt
Application already exist, are you sure you want to update the application (y/n)?
y
{
   "active" : 1,
   "command_line" : "ping www.google.com",
   "name" : "def",
   "pid" : -1,
   "return" : 0,
   "run_as" : "kfc",
   "working_dir" : "/opt"
}
```




## Remove a application
```
appc unreg -n abc
Are you sure you want to remove the application (y/n)?
y
Success
```

## Start a application
```
appc start -n def
```

## Stop a application
```
appc stop -n def
```
