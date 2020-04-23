class StringBuilder {
  constructor() {
    this.builder = new Array();
  }

  append(text) {
    this.builder.push(text);
    return this;
  }

  toString() {
    return this.builder.join('');
  }
}

module.exports = StringBuilder;