/*
* example-log-csv.js
*
* @Author: Erick Rettozi
*/

/*
* Import thorden
*/
var thorden = require('thorden');
var log = new thorden.Logger({
	Transports: {
		File: {
                	filename: './file.log', // Output
                        csv: true // (Comma-Separated values)
                },
		Console: {
			colorize: true // (info, error, warn)
		}
	}
});

var code = '110196009';
var login = 'login_test';
var domain = 'domaintest.com.br';

log.info('YourMethodName',code,login,domain,{dataInfo:'info message'});

log.error('YourMethodName',code,login,domain);

log.warn('YourMethodName',code,login,domain);

/*
* Destroy object and free memory
*/
log.destroy();

