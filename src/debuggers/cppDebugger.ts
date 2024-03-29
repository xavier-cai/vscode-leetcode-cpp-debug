import * as vscode from "vscode";
import * as fse from "fs-extra";
import * as path from "path";
import { Debugger } from "./debugger";
import { CodeIndentHelper } from "../util/codeIndentHelper";
import { IQuickItemEx } from "../util/ex";

interface IArgumentMetaInfo {
    type: string;
    name: string;
}

interface IFunctionMetaInfo {
    name: string;
    args: IArgumentMetaInfo[];
    type: string;
}

interface IProblemMetaInfo {
    name: string; // class name
    functions: IFunctionMetaInfo[];
    isDesignProblem: boolean;
    isInteractiveProblem: boolean;
}

export class CppDebugger extends Debugger {
    private readonly definition = "leetcode-definition.h";
    private readonly io = "leetcode-io.h";
    private readonly handler = "leetcode-handler.h";
    private readonly main = "leetcode-main.cpp";

    public async init(solutionEditor: vscode.TextEditor): Promise<string | undefined> {
        if (!solutionEditor || solutionEditor.document.isClosed || !this.codeTemplate) {
            return;
        }

        const insertContent: string = "#include \"" + this.definition + "\"";
        if (this.stubCodeHelper.getRaw().indexOf(insertContent) < 0) {
            this.stubCodeHelper.setFront(insertContent);
        }

        const dir: string = path.dirname(solutionEditor.document.uri.fsPath);
        const solution: string = path.basename(solutionEditor.document.uri.fsPath);

        const stub: string | undefined = await this.genStubCode(solution);
        if (!stub) {
            return;
        }
        this.stubFileHelper.files.push({ relative: this.handler, content: stub });

        return path.join(dir, this.main);
    }

    public async dispose(solutionEditor: vscode.TextEditor): Promise<void> {
    
    }

    private getMetaInfo(code: string): IProblemMetaInfo {
        const meta: IProblemMetaInfo = {
            name: '',
            functions: [],
            isDesignProblem: false,
            isInteractiveProblem: false
        }

        const classPattern: RegExp = /class +(Solution|[\w\d]+) *{?/;
        const initPattern: RegExp = / *([\w\d]+) *\(((?:[, ]*[\w\d<>, :\*]+[ \*&]+[\w\d]+)*)\)[ \{\}]*/;
        const funcPattern: RegExp = / *([\w\d<>, :\*]+) +([\w\d]+) *\(((?:[, ]*[\w\d<>, :\*]+[ \*&]+[\w\d]+)*)\)[ \{\}]*/;
        const argPattern: RegExp = / *([\w\d<>, :\*]+[&\* ]+)([\w\d]+) */;

        function getArgMetaInfo(arg: string): IArgumentMetaInfo {
            const match: RegExpExecArray | null = argPattern.exec(arg);
            if (match) {
                return { type: match[1].replace("&", "").trim(), name: match[2].trim() };
            }
            (function (): never {
                throw new Error(`Can not get meta info from ${arg}.`);
            })();
        }
        function getFuncMetaInfo(line: string): IFunctionMetaInfo | undefined {
            function normalize(type: string, name: string, args: string): IFunctionMetaInfo {
                const ret: IFunctionMetaInfo = {
                    name: name,
                    args: [],
                    type: type
                };
                if (args.replace(" ", "").length > 0) {
                    let parts: string[] = [];
                    let cnt: number = 0;
                    let index: number = 0;
                    let start: number = 0;
                    while (true) {
                        if (index >= args.length || (args[index] == ',' && cnt == 0)) {
                            parts.push(args.substr(start, index - start));
                            start = index + 1;
                            if (index >= args.length) {
                                break;
                            }
                        }
                        if (args[index] == '<') cnt += 1;
                        if (args[index] == '>') cnt -= 1;
                        index += 1;
                    }
                    ret.args = parts.map((value) => getArgMetaInfo(value));
                }
                return ret;
            }

            const eol = line.lastIndexOf(";");
            if (eol >= 0 && eol > line.lastIndexOf("}")) {
                return;
            }

            if (meta.name.length > 0) {
                const match: RegExpExecArray | null = initPattern.exec(line);
                if (match && match[1] == meta.name) {
                    return normalize('void', match[1].trim(), match[2]);
                }
            }
            const match: RegExpExecArray | null = funcPattern.exec(line);
            if (!match || match[1].trim().length <= 0) {
                return;
            }
            return normalize(match[1].trim(), match[2].trim(), match[3]);
        }

        const lines: string[] = code.split('\n');
        for (const line of lines) {
            const match: RegExpExecArray | null = classPattern.exec(line);
            if (match) {
                meta.name = match[1];
                meta.functions = [];
                meta.isDesignProblem = meta.name != 'Solution';
                continue;
            }
            if (meta.name.length > 0) {
                const func: IFunctionMetaInfo | undefined = getFuncMetaInfo(line);
                if (func) {
                    meta.functions.push(func);
                }
            }
        }
        return meta;
    }

    private async genStubCode(solution: string): Promise<string | undefined> {
        const meta: IProblemMetaInfo = this.getMetaInfo(this.codeTemplate);
        if (meta.name.length <= 0) {
            throw new Error("Invalid meta info.");
        }
        if (meta.isInteractiveProblem) {
            throw new Error("Unsupported problem type.");
        }
        if (meta.functions.length <= 0) {
            throw new Error("Can not find the entry function.");
        }

        function genArgsCode(func: IFunctionMetaInfo): string {
            return func.args.map((arg) => arg.name).join(", ");
        }
        function genInputCode(func: IFunctionMetaInfo, helper: CodeIndentHelper) {
            if (func.args.length <= 0) {
                return;
            }
            const tupleCode: string[] = [];
            for (const arg of func.args) {
                helper.line(arg.type + " " + arg.name + ";");
                tupleCode.push(arg.type + "&");
            }
            const tupleName: string = "__tuple__value";
            helper.line(`std::tuple<` + tupleCode.join(", ") + `> ${tupleName} { ` + genArgsCode(func) + ` };`);
            helper.line(`conv::FromJson(${tupleName}, in);`);
        }

        const code: CodeIndentHelper = new CodeIndentHelper();
        code.append(`#ifndef LEETCODE_HANDLER`)
            .line(`#define LEETCODE_HANDLER`)
            .line()
            .line(`#include "${solution}"`)
            .line(`#include "${this.io}"`)
            .line()
            .line("namespace lc {")
            .line()
            .line(`class Handler {`)
            .line(`public:`).right()
            .line(`static std::string GetClassName() { return "${meta.name}"; } `)
            .line(`Handler(const json::Json& in) {`).right();

        // generate constructor
        if (!meta.isDesignProblem) {
            code.line(`solution_ = new ${meta.name}();`);
        }
        else {
            const ctor: IFunctionMetaInfo | undefined = meta.functions.find((value: IFunctionMetaInfo) => value.name == meta.name && value.type == "void");
            if (ctor && ctor.args.length > 0) {
                genInputCode(ctor, code);
                code.line(`solution_ = new ${meta.name}(${genArgsCode(ctor)});`);
            }
            else {
                code.line(`solution_ = new ${meta.name}();`);
            }
        }
        code.left().line(`}`)
            .line(`~Handler() { delete solution_; }`);

        // generate handler function
        if (!meta.isDesignProblem) {
            const candidates: Array<IQuickItemEx<IFunctionMetaInfo>> = [];
            meta.functions.forEach(f => {
                //if ( f.type != "void" && f.args.length > 0) {
                    const args: string[] = f.args.map((a) => a.type);
                    candidates.push({
                        label: `> ${f.name}(${args.join(", ")}) => ${f.type}`,
                        value: f
                    });
                //}
            });
            if (candidates.length < 1) {
                throw new Error(`Can not find entry function in class [${meta.name}].`);
            }
            
            let func = candidates[0].value;
            if (candidates.length > 1) { // pick one
                const choice: IQuickItemEx<IFunctionMetaInfo> | undefined = await vscode.window.showQuickPick(candidates, {
                    placeHolder: "Please choose the entry function. (Press ESC to cancel)",
                    ignoreFocusOut: true,
                });
                if (!choice) {
                    return;
                }
                func = choice.value;
            }

            code.line(`json::Json Handle(const json::Json& in, const std::string& fname) { return json::Create<json::JNull>(); }`);
            code.line(`void Handle(io::SI& in, io::MO& out) {`).right();
            for (const arg of func.args) {
                code.line(`${arg.type} ${arg.name};`)
                    .line(`in >> ${arg.name};`);
            }
            code.line(`#ifdef LAZY_INTERACTION`)
                .line(`in.Input(LAZY_INTERACTION);`)
                .line(`#endif`);
            if (func.type != "void") {
                code.line(`out << solution_->${func.name}(${genArgsCode(func)}) << std::endl;`);
            }
            else {
                code.line(`solution_->${func.name}(${genArgsCode(func)});`);
                for (const arg of func.args) {
                    code.line(`out << ${arg.name} << std::endl;`);
                }
            }
            code.left().line('}');
        }
        else {
            code.line(`void Handle(io::SI& in, io::MO& out) {}`);
            code.line(`json::Json Handle(const json::Json& in, const std::string& fname) {`).right()
                .line(`if (fname == "") throw std::string("Empty function name.");`)
                .line(`#define CASE(func) else if (fname == #func)`);
            for (const func of meta.functions) {
                if (func.name == meta.name) {
                    continue;
                }
                code.line(`CASE (${func.name}) {`).right();
                genInputCode(func, code);
                const callCode: string = `solution_->${func.name}(${genArgsCode(func)})`;
                if (func.type == "void") {
                    code.line(`${callCode};`)
                        .line(`return json::Create<json::JNull>();`);
                }
                else {
                    code.line(`return conv::ToJson(${callCode});`);
                }
                code.left().line(`}`);
            }
            code.line(`#undef CASE`)
                .line(`throw std::string("Invalid function name.");`)
                .line(`return json::Create<json::JNull>();`)
                .left().line(`}`);
        }

        code.line().left().line(`private:`).right()
            .line(`${meta.name}* solution_;`)
            .left().line(`};`)
            .line()
            .line(`} // namespace lc`)
            .line();
        if (meta.isDesignProblem) {
            code.line(`#define SYSTEM_DESIGN`);
        }
        code.line(`#endif // LEETCODE_HANDLER`);
        return code.str();
    }
}
