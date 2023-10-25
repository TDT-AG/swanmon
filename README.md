# swanmon
swanmon is a small strongswan monitoring tool to retrieve runtime data as json via the [davici library](https://github.com/strongswan/davici).

## Usage
Available commands can be found with
```console
$ swanmon help
```
Currently only commands without user input are supported.

## Response Format
swanmon directly outputs the response values from the corresponding vici command as `"data"` (see [list of vici commands](https://github.com/strongswan/strongswan/blob/master/src/libcharon/plugins/vici/README.md#client-initiated-commands)), where streamed events are joined as an array.
```json
{"errors":[],"data":{}}
```
streamed command response:
```json
{"errors":[],"data":[]}
```

Errors are output with the `"errors"` key. There may technically be mulitple errors for the same command.
```json
{"errors":[{"message":"..."}],"data":[]}
```
### Example Responses
```console
$ swanmon version
```
```json
{"errors":[],"data":{"daemon":"charon","version":"5.9.11","sysname":"Linux","release":"5.15.135","machine":"mips"}}
```

```console
$ swanmon list-conns
```
```json
{"errors":[],"data":[{"test_connection":{"local_addrs":["%any"],"remote_addrs":["172.16.0.2"],"version":"IKEv1\/2","reauth_time":"10260","rekey_time":"0","unique":"UNIQUE_NO","dpd_timeout":"150","local-1":{"class":"pre-shared key","groups":[],"cert_policy":[],"certs":[],"cacerts":[]},"remote-1":{"class":"pre-shared key","id":"172.16.0.2","groups":[],"cert_policy":[],"certs":[],"cacerts":[]},"children":{"test_connection":{"mode":"TUNNEL","rekey_time":"3060","rekey_bytes":"0","rekey_packets":"0","dpd_action":"none","close_action":"none","local-ts":["192.168.100.0\/24"],"remote-ts":["192.168.101.0\/24"]},"another_test_connection":{"mode":"TUNNEL","rekey_time":"3060","rekey_bytes":"0","rekey_packets":"0","dpd_action":"none","close_action":"none","local-ts":["192.168.100.0\/24"],"remote-ts":["192.168.101.0\/24"]}}}}]}
```

_strongswan not started_
```json
{"errors":[{"message":"Connecting failed: Connection refused"}],"data":[]}
```
