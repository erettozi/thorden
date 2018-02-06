/*
* example-log-csv-fields.js
*
* @Author: Erick Rettozi
*/

/*
* Import thorden
*/
var thorden = require('thorden');
var log = new thorden.Logger({
	Transports: {
		File: [
			{
				level: 'info',
	                	filename: './file-info.log', // Output
        	                csv: true, // (Comma-Separated values)
				fields:['code','name','email','userJsonMetadata'], // Fields
                        	datePattern: true // format name: file.log-YYYY-MM-DD
			},
			{
				level: 'error',
	                	filename: './file-error.log', // Output
        	                csv: true, // (Comma-Separated values)
				fields:['email','name','code','userJsonMetadata'], // Fields
                        	datePattern: true // format name: file.log-YYYY-MM-DD
			},
			{
				level: 'warn',
	                	filename: './file-warn.log', // Output
				colorize: true // (info, error, warn)
			}
		],
		Console: [ 
			   {
				level: 'info',
				colorize: true
			   },
			   {
				level: 'error',
				colorize: true
			   },
			   {
				level: 'warn',
				colorize: true
			   }
		]
	}
});

var userData = {
     "firstName": "John",
     "lastName": "Smith",
     "age": 25,
     "address":
     {
         "streetAddress": "21 2nd Street",
         "city": "New York",
         "state": "NY",
         "postalCode": "10021"
     },
     "phoneNumber":
     [
         {
           "type": "home",
           "number": "212 555-1234"
         },
         {
           "type": "fax",
           "number": "646 555-4567"
         }
     ]
};

var logLine = {
        email:'jsmith@gmail.com',
        userJsonMetadata:userData,
        code:'110196009',
        name:'John Smith'
};

log.info('YourMethodName',logLine);

log.error('YourMethodName',logLine);

log.warn('YourMethodName','WARNING - code[%s] - email[%s] - name[%s] - userData[%j]',logLine.code,logLine.email,logLine.name,userData);

/*
* Destroy object and free memory
*/
log.destroy();

