import * as vscode from "vscode"
import * as fse from "fs-extra";

export class StubCodeHelper {
    private raw: string;
    private path: string;
    private front: string;
    private back: string;
    private installed: boolean;

    public constructor(doc: vscode.TextDocument) {
        this.raw = doc.getText();
        this.path = doc.uri.fsPath;
        this.front = "";
        this.back = "";
        this.installed = false;
    }
    public getRaw(): string {
        return this.raw;
    }
    public setFront(front: string): void {
        if (this.installed) {
            return;
        }

        this.front = front;
        if (front != "") {
            this.front += "\n";
        }
    }
    public setBack(back: string): void {
        if (this.installed) {
            return;
        }

        if (back != "") {
            this.back = "\n" + back;
        }
        else {
            this.back = back;
        }
    }
    private getDocumentEnd(editor: vscode.TextEditor): vscode.Position {
        return editor.document.lineAt(editor.document.lineCount - 1).range.end;
    }
    public async install(editor: vscode.TextEditor): Promise<void> {
        console.log("front=", this.front);
        console.log("back=", this.back);
        this.installed = true;
        if (!await editor.edit((e: vscode.TextEditorEdit) => {
            e.insert(new vscode.Position(0, 0), this.front);
            e.insert(this.getDocumentEnd(editor), this.back);
        })) {
            throw Error(`Can not edit the solution text.`);
        }
    }
    public async uninstall(editor: vscode.TextEditor): Promise<void> {
        if (this.installed) {
            await editor.edit((e: vscode.TextEditorEdit) => {
                const front: vscode.Range = new vscode.Range(
                    new vscode.Position(0, 0),
                    editor.document.positionAt(this.front.length)
                );
                if (editor.document.getText(front) == this.front) {
                    e.delete(front);
                }
                const end: vscode.Position = this.getDocumentEnd(editor);
                const back: vscode.Range = new vscode.Range(
                    editor.document.positionAt(editor.document.offsetAt(end) - this.back.length),
                    end
                );
                if (editor.document.getText(back) == this.back) {
                    e.delete(back);
                }
            });
        }
    }
}