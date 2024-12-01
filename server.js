const express = require("express");
const bodyParser = require("body-parser");
const mongoose = require("mongoose");

// MongoDB Atlas connection string
const mongoURI = "mongodb+srv://admin:pass@123@esp32.tfbcc.mongodb.net/?retryWrites=true&w=majority&appName=ESP32";

// Connect to MongoDB
mongoose.connect(mongoURI, {
    useNewUrlParser: true,
    useUnifiedTopology: true,
})
.then(() => console.log('Connected to MongoDB'))
.catch((error) => console.error('Error connecting to MongoDB:', error));

// Create a schema for storing the data
const DataSchema = new mongoose.Schema({
    ldr: Number,
    pir: Number,
    vib: Number,
    button: Number,
    timestamp: { type: Date, default: Date.now }
});

// Create a model for the schema
const DataModel = mongoose.model('Data', DataSchema);

// Initialize Express app
const app = express();

// Use body-parser to handle incoming JSON data
app.use(bodyParser.json());

// POST endpoint to receive data from ESP32
app.post('/api/data', async (req, res) => {
    const { ldr, pir, vib, button } = req.body;

    try {
        // Create a new document based on the received data
        const newData = new DataModel({ ldr, pir, vib, button });
        
        // Save the data to MongoDB
        await newData.save();
        
        // Send a response indicating success
        res.status(201).send({ message: 'Data saved successfully' });
    } catch (error) {
        console.error('Error saving data:', error);
        res.status(500).send({ error: 'Failed to save data' });
    }
});

// Start the server
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});

