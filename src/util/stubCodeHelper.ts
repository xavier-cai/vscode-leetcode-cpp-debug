import * as vscode from "vscode"
import * as fse from "fs-extra";

export class StubCodeHelper {
    private raw: string;
    private text: string | undefined;
    private path: string;
    public constructor(doc: vscode.TextDocument) {
        this.raw = doc.getText();
        this.text = undefined;
        this.path = doc.uri.fsPath;
    }
    public getRaw(): string {
        return this.raw;
    }
    public setText(text: string | undefined): void {
        this.text = text;
    }
    public async install(): Promise<void> {
        if (this.text) {
            await fse.writeFile(this.path, this.text);
        }
    }
    public async uninstall(): Promise<void> {
        await fse.writeFile(this.path, this.raw);
    }
}