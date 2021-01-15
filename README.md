# Welcome to the SyncSix Prop Controllers Github!
Here you will find various Arduino Sketches that can run on the SyncSix line of open source prop controllers (as well as other resources). 

Current factory default version is: SyncSix_V1.ino


You can define the controller quantization time constant "quant" to be whatever you like, the deafualt is 75(ms). This is how long the controller will wait to update the outputs. A shorter 'quant' value will make the controller more responsive but reduce its maximum sequence length (though this shouldnt be an issue, at default 75ms the controller can record a sequence of 1h20min).  
