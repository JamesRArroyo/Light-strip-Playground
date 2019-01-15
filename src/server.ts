import app from "./app";
const PORT = process.env.PORT || 8266;

app.listen(PORT, () => {
    console.log(`ESP-Light-Strip app listening on port ${PORT}!`);
})