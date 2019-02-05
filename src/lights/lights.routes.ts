import { Request, Response } from "express";
import * as mqtt from 'mqtt';


export class Lights {

  public routes(app): void {


    // app.route('/').get((req: Request, res: Response) => {
    //   res.sendFile(path.join(__dirname + '/index.html'));
    // })

    app.route('/lights/log').post((req: Request, res: Response) => {
      console.log('**********************')
      console.log(req.body);
      res.status(200).send({'message': 'done!'});
    })


    app.route('/control-lights').get((req: Request, res: Response) => {
      var client = mqtt.connect('http://localhost:1883')
      console.log(req.query)
      let topics = [];
      let lightConfig = {
        state: '',
        color: { r: 0, g: 0, b: 255 },
        brightness: '',
        effect: ''
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

      if (req.query.effect) {
        lightConfig.effect = req.query.effect;
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

      client.on('connect', () => {
        topics.forEach(topic => {
          const topicObj = {};
          topicObj[topic] = 0;
          client.publish(topic, msg, {qos: 1})
        });

      })

      

      res.status(200).send({'message': 'done!'});

    })



    app.route('/topics').get((req: Request, res: Response) => {
      /* Temp spot to store light groups */
      const topicGroups = [
        {
          name: 'ALL', topics: ['light1', 'light2', 'light3', 'light4', 'light5', 'light6',
            'light7', 'light8', 'light9', 'light10', 'light11', 'light12', 'light13', 'light14', 
            'light15', 'light16', 'light17', 'light18', 'light20', 'light19', 'light21', 
            'light22']
        },
        {
          name: 'Desks', topics: ['light17', 'light18', 'light19']
        },
        {
          name: 'Elevator', topics: ['light5', 'light6']
        },
        {
          name: 'Clocks', topics: ['light11', 'light12', 'light13', 'light14', 'light15', 'light16']
        },
        { 
          name: 'door', topics: ['light21'] 
        },
        {
          name: 'insideE', topics: ['light20']
        }
      ];
      res.send(topicGroups)
    })







   
 

  }
}