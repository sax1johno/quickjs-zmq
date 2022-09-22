/*
MIT License

Copyright (c) 2022 John O'Connor (sax1johno@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

import * as zmq from './quickjs-zmq.so'
import { setTimeout } from 'os';

/**
 * Note: This class is not currently functional.
 */
export class Zmq {
    static version() {
        return zmq.version();
    }
}

// Constants
export const ZMQ_PAIR=0
export const ZMQ_PUB=1
export const ZMQ_SUB=2
export const ZMQ_REQ=3
export const ZMQ_REP=4
export const ZMQ_DEALER=5
export const ZMQ_ROUTER=6
export const ZMQ_PULL=7
export const ZMQ_PUSH=8
export const ZMQ_XPUB=9
export const ZMQ_XSUB=10
export const ZMQ_STREAM=11


export class Socket {
    // static context = zmq.createContext();
    static errorCodeToString(errorCode) {
        return zmq.strerror(errorCode);
    }

    static formatError(errorCode) {
        return {"rc": errorCode, "description": Socket.errorCodeToString(errorCode)}
    }

    constructor(type=ZMQ_REP) {
        this.socket = zmq.zsock_new(type);
        // console.log(this.socket);
        // Set up event listeners.
        this.listeners = {
            connected: [],
            disconnected: [],
            data: [],
            error: [] 
        };
        this.listening = false;
    }

    on(event, cb) {
        // console.log(`Attaching callback to ${event}`);
        this.listeners[event] = this.listeners[event] || [];
        this.listeners[event].push(cb);
        // console.log(this.listeners[event].length);
    }

    destroy() {
        this.emit("disconnected", this.socket);
        zmq.zsock_destroy(this.socket);
        // this.socket = undefined;
        // this.context = undefined;
        this.listening = false;
    }

    emit(event, data={}) {
        // console.log(`Emitting ${event}`);
        if (!this.listeners[event]) return;
        // console.log(`Number of listeners for ${event} is ${this.listeners[event].length}`);
        for (var i = 0; i < this.listeners[event].length; i++) {
            // console.log(this.listeners[event][i]);
            this.listeners[event][i](data);
        }
    }

    unbind(address) {
        return new Promise((resolve, reject) => {
            // CZMQ_EXPORT int
            // zsock_unbind (zsock_t *self, const char *format, ...) CHECK_PRINTF (2);     
            var rc = zsock_unbind(this.socket, address);
            if (rc < 0) {
                reject(`Failed with return code ${rc}`);
            } else {
                resolve();
            }
        })
    }

    bind(address) {
        return new Promise((resolve, reject) => {
            var boundPort = zmq.zsock_bind(this.socket, address);
            if (boundPort >= 0) {
                this.emit("connected", {"port": "boundPort"});
                this.port = boundPort;
                resolve(returnValue);
            } else {
                this.emit("error", Socket.formatError(returnValue));
                reject(returnValue);
            }
        });
    }

    async listen(address) {
        // console.log("Inside listen");
        this.port = await this.bind(address);
        // console.log(`Socket bind returned with ${returnCode}`);
        // if (returnCode < 0) {
        //     throw
        // }
        // console.log("Socket is bound");
        this.listening = true;
        while (this.listening) {
            // console.log("Listening");
            try {
                // console.log("Receive");
                var data = zmq.zsock_recv(this.socket);
                // console.log(`Data: ${data}`);
                this.emit("data", data);
                // zmq.zsock_send(this.socket, "OK");
                // console.log("After data");
            } catch (e) {
                console.log(e);
                this.destroy();
            }
        }
    }

    connect(address) {
        return new Promise((resolve, reject) => {
            console.log("Attempting to connect socket.")
            var returnValue = zmq.zsock_connect(this.socket, address);
            console.log(`Socket connected returned with ${returnValue}`);
            if (returnValue == 0) {
                this.emit("connected");
                resolve(returnValue);
            } else {
                this.emit("error", Socket.formatError(returnValue));
                reject(returnValue);
            }
        });
    }

    send(message) {
        return new Promise((resolve, reject) => {
            // console.log("In send");
            console.log(`Sending ${JSON.stringify(message)}`);
            var returnValue = zmq.zsock_send(this.socket, JSON.stringify(message));
            if (returnValue >= 0) {
                // this.emit("data");
                // 100ms timeout gives libzmq time to flush the message queue
                // Ensures that quick running clients still send message.
                setTimeout(() => {
                    resolve(returnValue);
                }, 100);
            } else {
                this.emit("error", Socket.formatError(zmq_errno()));
                reject(returnValue);
            }
        });
    }
    
}