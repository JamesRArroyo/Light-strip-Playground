import { Request, Response } from "express";
import * as fs from 'fs';
import * as path from 'path';
import * as mqtt from 'mqtt';


export class Lights {

  public routes(app): void {


    app.route('/').get((req: Request, res: Response) => {
      res.sendFile(path.join(__dirname + '/index.html'));
    })

    app.route('/control-lights').get((req: Request, res: Response) => {
      var client = mqtt.connect('http://localhost:1883')
      console.log(req.query)
      let topics = [];
      let lightConfig = {
        state: '',
        color: { r: 0, g: 0, b: 255 },
        brightness: '',
        effect: 'solid'
      };

      if (req.query.topics) {
        topics = req.query.topics.split(', ');
      }

      if (req.query.state) {
        lightConfig.state = req.query.state;
      }
      
      if (req.query.brightness) {
        lightConfig.brightness = req.query.brightness;
      }

      if (req.query.color) {
        const color = req.query.color;
        if (color === 'red') {
          lightConfig.color = { r: 255, g: 0, b: 0 };
        } else if (color === 'green') {
          lightConfig.color = { r: 0, g: 255, b: 0 };
        } else if (color === 'blue') {
          lightConfig.color = { r: 0, g: 0, b: 255 };
        } else if (color === 'aqua') {
          lightConfig.color = { r: 0, g: 255, b: 255 };
        } else if (color === 'white') {
          lightConfig.color = { r: 255, g: 255, b: 255 };
        }
      }


      let msg = JSON.stringify(lightConfig);
      console.log(topics);
      console.log(msg);
      client.on('connect', () => {
        topics.forEach(topic => {
          client.subscribe(topic, (err) => {
            if (!err) {
              console.log('publishing msg...')
              client.publish(topic, msg)
            } else {
              console.log(err);
            }
          });
        });

      })

      res.send('DONE!')
    })



    app.route('/topics').get((req: Request, res: Response) => {
      const topicGroups = [
        {name: 'ALL', topics: ['stage/left/elevator/elevator1', 'stage/left/elevator/elevator2', 'stage/right/clock/clock3']},
        {name: 'Elevator', topics: ['stage/left/elevator/elevator1', 'stage/left/elevator/elevator2']},
        {name: 'Clock', topics: ['stage/right/clock/clock3']}
      ];
      res.send(topicGroups)
    })







   
 

  }
}