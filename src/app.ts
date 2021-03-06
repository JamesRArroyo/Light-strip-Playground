import * as express from 'express';
import * as bodyParser from 'body-parser';
import { FirmwareUpdates } from "./firmwareUpdate/firmwareUpdate.routes";
import { Lights } from './lights/lights.routes';

class App {

    public app: express.Application;
    public firmwareUpdateRoutes: FirmwareUpdates = new FirmwareUpdates();
    public lightRoutes: Lights = new Lights();

    constructor() {
        this.app = express();
        this.config();

        this.app.use(function(req, res, next) {
            res.header("Access-Control-Allow-Origin", "*");
            res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
            next();
          });

        this.firmwareUpdateRoutes.routes(this.app);
        this.lightRoutes.routes(this.app);
    }

    private config(): void {
        this.app.use(bodyParser.json());
        this.app.use(bodyParser.urlencoded({
            extended: false
        }));
    }

}

export default new App().app;