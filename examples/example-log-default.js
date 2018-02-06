/*
* example-default.js
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
                        colorize: true // (info, error, warn)
                },
		Console: {
			colorize: true // (info, error, warn)
		}
	}
});

let code = '110196009';
let login = 'login_test';
let domain = 'domaintest.com.br';

log.info('YourMethodName','INFO Test - code[%s] - login[%s] - domain[%s] - json[%j]',code,login,domain,{dataInfo: 'info message'});

log.error('YourMethodName','ERROR Test - code[%s] - login[%s] - domain[%s] - json[%j]',code,login,domain,{dataError: 'error message'});

log.warn('YourMethodName','WARNING Test - code[%s] - login[%s] - domain[%s] - json[%j]',code,login,domain,{dataWarning: 'warning message'});

/*
* Destroy object and free memory
*/
log.destroy();

