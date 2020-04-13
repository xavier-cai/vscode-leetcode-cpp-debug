export class CodeIndentHelper {
    private count: number;
    private codes: string[];
    constructor(private indent: string = "    ") {
        this.count = 0;
        this.codes = [""];
    }
    public line(code: string = ""): CodeIndentHelper {
        let init: string = "";
        for (var i = 0; i < this.count; ++i) {
            init += this.indent;
        }
        this.codes.push(init + code);
        return this;
    }
    public right(): CodeIndentHelper {
        this.count += 1;
        return this;
    }
    public left() : CodeIndentHelper {
        this.count -= 1;
        return this;
    }
    public append(code: string): CodeIndentHelper {
        this.codes[this.codes.length - 1] += code;
        return this;
    }
    public str(): string {
        return this.codes.join('\n');
    }
}