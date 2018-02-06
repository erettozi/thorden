/*
* example-log-csv.js
*
* @Author: Erick Rettozi
*/

/*
* Import thorden
*/
const thorden = require('thorden');
const log = new thorden.Logger({
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

let code = '110196009';
let login = 'login_test';
let domain = 'domaintest.com.br';

log.info('YourMethodName',code,login,domain,{dataInfo:'info message'});

log.error('YourMethodName',code,login,domain);

log.warn('YourMethodName',code,login,domain);

/*
* Destroy object and free memory
*/
log.destroy();

