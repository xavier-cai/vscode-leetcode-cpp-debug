import { Language } from "./config";
import * as fse from "fs-extra";
import * as path from "path"
import * as vscode from "vscode"
import * as uc from "./config"
import * as glob from "glob"
import * as iconv from "iconv-lite"

export interface StubFile {
    relative: string;
    content: string;
}

export class StubFileHelper {
    public files: StubFile[];
    constructor(private token: string, public resourcesPath: string, private encoding = "utf8") {
        this.files = [];
        this.validate();
    }
    private validate() {
        try {
            iconv.encode("", this.encoding);
        }
        catch {
            throw new Error(`Encoding not recognized: '${this.encoding}'`);
        }
    }
    private async check(filePath: string): Promise<boolean> {
        return (iconv.decode(await fse.readFile(filePath), this.encoding)).indexOf(this.token) >= 0;
    }
    private async foreach(dir: string, cb: (stub: StubFile, filePath: string, exists: boolean) => void) {
        for (const stub of this.files) {
            const file: string = path.join(dir, stub.relative);
            const exists: boolean = fse.existsSync(file);
            if (exists) {
                if (!await this.check(file)) {
                    throw new Error(`Token missed in file: ${file}`);
                }
            }
            cb(stub, file, exists);
        }
    }
    public async install(dir: string) {
        await this.foreach(dir, async (stub, file) => {
            await fse.writeFile(file, iconv.encode(this.token + "\n\n" + stub.content, this.encoding));
        });
    }
    public async uninstall(dir: string) {
        await this.foreach(dir, async (stub, file, exists) => {
            if (exists) {
                await fse.remove(file);
            }
        });
    }
}

class StubFileHelperFactory {
    private context: vscode.ExtensionContext | undefined;
    public initialize(context: vscode.ExtensionContext) {
        this.context = context;
    }
    public async create(language: Language, filePath: string): Promise<StubFileHelper | undefined> {
        if (!this.context) {
            return;
        }
        const res: string = this.context.asAbsolutePath(path.join(uc.configs.resourcesPath, uc.configs.codePath, language.name));
        if (!await fse.pathExists(res)) {
            return;
        }
        //const fileName: string = path.basename(filePath);
        //const oEncoding: string = (/.*[\u4e00-\u9fa5]+.*$/.test(fileName)) ? "gbk" : "utf8";
        const oEncoding: string | undefined = uc.getEncoding();
        const stubFileHelper = new StubFileHelper(language.commet.begin + ' ' + uc.configs.token + ' ' + language.commet.end, res, oEncoding ? oEncoding : "utf8");
        const files: string[] = glob.sync(language.stub, {
            cwd: res,
            nodir: true
        });
        for (const file of files) {
            stubFileHelper.files.push({
                relative: file,
                content: await fse.readFile(path.join(res, file), "utf8")
            })
        }
        return stubFileHelper;
    }
}

export const stubFileHelperFactory = new StubFileHelperFactory();