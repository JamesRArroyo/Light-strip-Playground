import { Request, Response } from "express";
import * as fs from 'fs';
import * as path from 'path';


export class FirmwareUpdates {
    firmwareVersionNumber:string = '38'


    public routes(app): void {


        app.route('/update_firmware/version').get((req: Request, res: Response) => {
            res.status(200).send(this.firmwareVersionNumber);
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