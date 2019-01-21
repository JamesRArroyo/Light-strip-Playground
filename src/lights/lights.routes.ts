import { Request, Response } from "express";
import * as fs from 'fs';
import * as path from 'path';
import * as mqtt from 'mqtt';


export class Lights {

    public routes(app): void {


        app.route('/').get((req: Request, res: Response) => {
            res.sendFile(path.join(__dirname+'/index.html'));
        })

        app.route('/test-mqtt-blue').get((req: Request, res: Response) => {
            var client = mqtt.connect('http://localhost:1883')

            client.on('connect', () => {
                client.subscribe('bruh/porch', (err) => {
                    if (!err) {
                        console.log('publishing msg...')
                        const msg = '{"state":"ON","color":{"r":0,"g":0,"b":255},"brightness":255,"effect":"solid"}'
                        client.publish('bruh/porch/set', msg)
                    } else {
                        console.log(err);
                    }
                })
            })

            res.redirect('http://localhost:1883/')
        })

        app.route('/test-mqtt-green').get((req: Request, res: Response) => {
            var client = mqtt.connect('http://localhost:1883')

            client.on('connect', () => {
                client.subscribe('bruh/porch', (err) => {
                    if (!err) {
                        console.log('publishing msg...')
                        const msg = '{"state":"ON","color":{"r":0,"g":255,"b":0},"brightness":255,"effect":"solid"}'
                        client.publish('bruh/porch/set', msg)
                    } else {
                        console.log(err);
                    }
                })
            })

            res.redirect('http://localhost:1883/')
        })

        app.route('/test-mqtt-red').get((req: Request, res: Response) => {
            var client = mqtt.connect('http://localhost:1883')

            client.on('connect', () => {
                client.subscribe('bruh/porch', (err) => {
                    if (!err) {
                        console.log('publishing msg...')
                        const msg = '{"state":"ON","color":{"r":255,"g":0,"b":0},"brightness":255,"effect":"solid"}'
                        client.publish('bruh/porch/set', msg)
                    } else {
                        console.log(err);
                    }
                })
            })

            res.redirect('http://localhost:1883/')
        })


        app.route('/mqtt-list-clients').get((req: Request, res: Response) => {
            var client = mqtt.connect('http://localhost:1883')

            client.on('connect', function() { // Check you have a connection
                // Subscribe to a Topic
                    client.subscribe("bruh/porch", function(e, m) {
                        console.log('Subscribed ', e, m);
                        client.on('message', function(topic, message, packet) {
                            
                        console.log(topic);
                        console.log(message);
                        console.log(packet);
                                
                        });
                    });
                });
                
                client.on('reconnect', () => {
                    console.log('Reconnecting-----------');
                 })
                client.on('close', () => {
                    console.log('Closing client');
                })
                client.on('offline', () => {
                    console.log('Client is gone offline');
                });
                
                client.on('error', (e) => {
                    console.log('Error in connecting to mqtt broker ', e);
                    client.end();
                })

            res.send('http://localhost:1883/')
        })

        app.route('/update_firmware')
            .all((req, res, next) => {
                const user_agent = req.headers['user-agent']
                console.log('Time: ', Date.now())
                console.log("User-Agent:" + user_agent);
                next()
            })
            .get((req: Request, res: Response) => {
                console.log('Starting Update')

                const path_to_file = path.resolve(__dirname, "../ArduinoCode/ArduinoCode.ino.nodemcu.bin")
                fs.readFile(path_to_file, "binary", (error: Error, data: string | Buffer) => {
                    if (error) {
                        console.log(error)
                        res.writeHead(500, { "Content-Type": "text/plain" });
                        res.write(error + "\n");
                        res.end();
                        return;
                    }


                    console.log('Sending Update...')
                    //Get File Size
                    var stats = fs.statSync(path_to_file);
                    res.setHeader("Content-Type", "text/html");
                    res.setHeader("Content-Disposition", "attachment");
                    res.setHeader("Content-Length", stats.size.toString());
                    res.write(data, "binary");
                    res.end();
                })
            })
    }
}