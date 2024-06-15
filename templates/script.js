let isStreaming = false;
    function startStream() {
        const button = document.getElementById('startStream');
        var videoElement = document.getElementById("videoStream");
        var card = document.getElementById("card1");
        videoElement.style.display = (videoElement.style.display === 'none' || videoElement.style.display === '') ? 'block' : 'none';
        card.style.display = (card.style.display === 'none' || card.style.display === '') ? 'block' : 'none';
        //button.innerHTML = isStreaming ? 'Start Stream' : 'Stop Stream';
        isStreaming = !isStreaming;
    }
var video = document.getElementById('videoStream');
    video.src = ""; // This should match the path configured in the ESP32-CAM server
var isSpeaking = false;
function speakText(text) {
       const synth = window.speechSynthesis;
        const utterance = new SpeechSynthesisUtterance(text);

        // Set the isSpeaking variable when speaking starts
        utterance.onstart = function() {
            isSpeaking = true;

            // Mute the microphone when Web Speech API is speaking
            if (isRecording) {
                speechRecognition.stop();
            }
        };
        // Clear the isSpeaking variable when speaking ends
        utterance.onend = function() {
            isSpeaking = false;
            // Unmute the microphone when Web Speech API speaking ends
            if (isRecording) {
                speechRecognition.start();
            }
        };
        synth.speak(utterance);
        }

//webkit speech
$(document).ready(function() {
        const transcriptionElement = document.getElementById('transcription');
        const startButton = document.getElementById('startButton');
        const stopButton = document.getElementById('stopButton');
        const canvas = document.getElementById('frequencyCanvas');
        const ctx = canvas.getContext('2d');
        var speechRecognition = new webkitSpeechRecognition();
        speechRecognition.continuous = true;
        speechRecognition.interimResults = true;
        let isRecording = false;

        speechRecognition.onresult = function(event) {
            let interimTranscript = '';
            for (let i = event.resultIndex; i < event.results.length; i++) {
                if (event.results[i].isFinal) {
                    let finalTranscript = event.results[i][0].transcript;
                    transcriptionElement.innerHTML = finalTranscript;
                    $.ajax({
                        type: 'POST',
                        url: '/process_audio',
                        data: JSON.stringify({ audio: finalTranscript }),
                        contentType: 'application/json',
                        success: function(response) {
                            speakText(response);
                        }
                    });
                } else {
                    interimTranscript += event.results[i][0].transcript;
                }

             }
        };

        startButton.addEventListener('click', function() {
            if (!isRecording) {
                speechRecognition.start();
            }
            isRecording = true;
        });

        stopButton.addEventListener('click', function() {
            speechRecognition.stop();
            isRecording = false;
        });

        // Optionally, you can add a confirmation dialog before stopping recording
        window.onbeforeunload = function() {
            if (isRecording) {
                return 'Are you sure you want to stop recording?';
            }
        };
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        const analyser = audioContext.createAnalyser();
        analyser.fftSize = 256;
        const bufferLength = analyser.frequencyBinCount;
        const dataArray = new Uint8Array(bufferLength);
        navigator.mediaDevices.getUserMedia({ audio: true })
            .then(function(stream) {
                const source = audioContext.createMediaStreamSource(stream);
                source.connect(analyser);
            })
            .catch(function(err) {
                console.error('Error accessing microphone:', err);
            });
        startButton.addEventListener('click', function() {
            audioContext.resume().then(function() {
                if (!isRecording) {
                speechRecognition.start();
                }
                isRecording = true;
                visualize();
            });
        });

        stopButton.addEventListener('click', function() {
            speechRecognition.stop();
            isRecording = false;
        });

        function visualize() {
         analyser.getByteFrequencyData(dataArray);
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        const barWidth = (canvas.width / bufferLength) * 2.5;
        let x = 0;

        for (let i = 0; i < bufferLength; i++) {
            let barHeight = dataArray[i];

            if (isSpeaking) {
                // If Web Speech API is speaking, set bar height to a fixed value or adjust as needed
                barHeight = 50; // Example: Set a fixed height during speech
            }

            ctx.fillStyle = `#6BD6E1`;
            ctx.fillRect(x, canvas.height - barHeight / 2, barWidth, barHeight / 2);
            x += barWidth + 1;
        }

        requestAnimationFrame(visualize);
    }
    // Call the initial visualization
    visualize();

    });
    var isFrequencyBarVisible = false;

    function toggleFrequencyBar() {
      var frequencyBarContainer = document.querySelector('.frequency-bar-container');
      var stopButton = document.querySelector('.stopButton');
      isFrequencyBarVisible = !isFrequencyBarVisible;
      if (isFrequencyBarVisible) {
        frequencyBarContainer.style.display = 'block';
        stopButton.style.display = 'block';
      } else {
        frequencyBarContainer.style.display = 'none';
        stopButton.style.display = 'none';
      }
    }
    function toggleOutline() {
            var outline1 = document.getElementById('outline1');
            var outline2 = document.getElementById('outline2');

            // Toggle the visibility of the outline elements
            outline1.style.display = (outline1.style.display === 'none' || outline1.style.display === '') ? 'block' : 'none';
            outline2.style.display = (outline2.style.display === 'none' || outline2.style.display === '') ? 'block' : 'none';


        }