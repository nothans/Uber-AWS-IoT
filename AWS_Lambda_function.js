var AWS = require('aws-sdk');
var https = require('https');

// Set options for AWS IoT
//   - make sure credentials have AWSIoTDataAccess permissions policy
var CONFIG_AWSIOT = {
  endpoint: 'XXX.iot.us-east-1.amazonaws.com',
  region: 'us-east-1',
  accessKeyId: 'XXX',
  secretAccessKey: 'XXX'
};

// Set options for Uber API  
var CONFIG_UBER = {
  uber_token: "XXX",
  product_id: "XXX",
  start_latitude: 42.300080,
  start_longitude: -71.350349
};

function callUber(event, context) {
    
  var data =  {
    product_id:      CONFIG_UBER.product_id,
    start_latitude:  CONFIG_UBER.start_latitude,
    start_longitude: CONFIG_UBER.start_longitude
  };
  
  data = JSON.stringify(data);
  
  var headers = {
    'Authorization': 'Bearer ' + CONFIG_UBER.uber_token,
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(data)
  };

  var options = {
    host:    'api.uber.com',
    path:    '/v1/requests/estimate',
    method:  'POST',
    headers: headers
  };

  var req = https.request(options, function(res) {
      
    res.on('data', function (chunk) {
        
      uberData = JSON.parse(chunk);
      
      iotdata = new AWS.IotData(CONFIG_AWSIOT);
  
      params = {
        topic: 'uber_response',
        payload: uberData.pickup_estimate.toString(),
        qos: 0
      };
      
      iotdata.publish(params, function(err, data) {
        if (err) {
          console.log(err, err.stack);
          context.fail(event); 
        }
        else {
          console.log(data);
          context.succeed(event);
        }
      });
  
      console.log(chunk.toString());
    });

  });

  req.write(data);
  req.end();
  
  req.on('error', function(e) {
    context.fail(event); 
  });
  
}

exports.handler = callUber;
