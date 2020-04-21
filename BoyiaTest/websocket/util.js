const os = require('os');

const util = {
  getLocalIP() {
    const interfaces = os.networkInterfaces();
    for (const devName of Object.keys(interfaces)) {
      if (devName.toLowerCase().indexOf('loopback') !== -1) {
        continue;
      }
      const iface = interfaces[devName];
      for (let i = 0; i < iface.length; i++) {
        const alias = iface[i];
        if (alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal) {
          return alias.address;
        }
      }
    }

    return '127.0.0.1';
  }

}

module.exports = util;