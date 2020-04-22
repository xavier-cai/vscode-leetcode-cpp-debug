import * as vscode from "vscode";
import { leetCodeDebugger } from "../leetCodeDebugger";

export async function debugSolution(uri?: vscode.Uri): Promise<void> {
    let textEditor: vscode.TextEditor | undefined;
    if (uri) {
        textEditor = await vscode.window.showTextDocument(uri, { preview: false });
    } else {
        textEditor = vscode.window.activeTextEditor;
    }

    if (!textEditor) {
        return;
    }

    try {
        await leetCodeDebugger.startDebugging(textEditor.document.uri.fsPath);
    } catch (error) {
        if (error instanceof Error) {
            await vscode.window.showInformationMessage(`${error}`);
        }
        else {
            await vscode.window.showInformationMessage(`Error: ${error}`);
        }
        return;
    }
}
