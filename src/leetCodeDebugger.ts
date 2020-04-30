import * as vscode from "vscode";
import * as fse from "fs-extra";
import * as path from "path";
import { IDebuggerConstructor, getDebugger } from "./debuggers/debuggerManager";
import * as uc from "./util/config";
import { problemFetcher } from "./util/problemFetcher";
import { StubFileHelper, stubFileHelperFactory } from "./util/stubFileHelper";
import { StubCodeHelper } from "./util/stubCodeHelper"

class LeetCodeDebugger {
    public async startDebugging(solutionFilePath: string): Promise<void> {
        const language: uc.Language | undefined = uc.languageHelper.getByPath(solutionFilePath);
        if (!language) {
            vscode.window.showInformationMessage(`Unsupport file type.`);
            return;
        }

        const ctor: IDebuggerConstructor | undefined = getDebugger(language.name);
        if (!ctor) {
            vscode.window.showInformationMessage(`Unsupport language: ${language.name}.`);
            return;
        }

        const source: uc.Source | undefined = uc.getSource();
        if (!source) {
            vscode.window.showInformationMessage(`Unknown source.`);
            return;
        }

        let codeTemplate: string = "";
        if (source == uc.Source.Local) {
            codeTemplate = await fse.readFile(solutionFilePath, "utf8");
        }
        else { // fetch problem online
            const id: string | undefined = uc.getProblemId(solutionFilePath);
            if (!id) {
                vscode.window.showInformationMessage(`Regular expression non-matched.`);
                return;
            }
            try {
                codeTemplate = await vscode.window.withProgress<string>({
                        location: vscode.ProgressLocation.Notification
                    }, async (p: vscode.Progress<{}>) => {
                        return new Promise(async (resolve, reject): Promise<void> => {
                            p.report({ message: `Fetching problem...` });
                            try {
                                resolve(await problemFetcher.fetchProblem(id, language));
                            } catch (e) {
                                reject(e);
                            }
                        });
                    }
                );
            }
            catch (e) {
                vscode.window.showInformationMessage(`Fetch problem failed: ${e}`);
            }
        }

        const stubFileHelper: StubFileHelper | undefined = await stubFileHelperFactory.create(language, solutionFilePath);
        if (!stubFileHelper) {
            vscode.window.showInformationMessage(`Can not create entry code.`);
            return;
        }

        const solutionDocument: vscode.TextDocument = await vscode.workspace.openTextDocument(solutionFilePath);
        const stubCodeHelper: StubCodeHelper = new StubCodeHelper(solutionDocument);

        const debuggerInstance = new ctor(codeTemplate, stubCodeHelper, stubFileHelper);
        async function switchEditor(filePath: string): Promise<vscode.TextEditor> {
            const textDocument: vscode.TextDocument = await vscode.workspace.openTextDocument(filePath);
            return await vscode.window.showTextDocument(textDocument, undefined, true);
        }
        let listenEvent: vscode.Disposable | undefined;
        async function afterDebugging(): Promise<void> {
            const editor = await switchEditor(solutionFilePath);
            await debuggerInstance.dispose(editor);
            if (uc.getIsDeleteTemporaryContent()) {
                const stub: Promise<void>[] = [
                    stubFileHelper?.uninstall(path.dirname(editor.document.uri.fsPath)),
                    stubCodeHelper.uninstall()
                ]
                await Promise.all(stub);
            }
            await editor.document.save();
            if (listenEvent) {
                listenEvent.dispose();
            }
        }
        try {
            const solutionEditor: vscode.TextEditor = await vscode.window.showTextDocument(solutionDocument);
            const debugEntry: string | undefined = await debuggerInstance.init(solutionEditor);
            if (!debugEntry) {
                await afterDebugging();
                return;
            }

            const stub: Promise<void>[] = [
                stubCodeHelper.install(),
                stubFileHelper.install(path.dirname(solutionEditor.document.uri.fsPath))
            ];
            await Promise.all(stub);

            if (!await fse.pathExists(debugEntry)) {
                await afterDebugging();
                return;
            }

            let entryEditor: vscode.TextEditor | undefined;
            if (debugEntry) {
                entryEditor = await switchEditor(debugEntry);
            }

            listenEvent = vscode.debug.onDidTerminateDebugSession(async () => { await afterDebugging(); });
            //await solutionEditor.document.save();
            if (!await this.launch()) {
                await afterDebugging();
            }

            if (entryEditor) {
                entryEditor.hide();
            }
        }
        catch (error) {
            vscode.window.showInformationMessage(`Failed to start debugging: ${error}`);
            await afterDebugging();
        }
    }

    private async launch(): Promise<boolean> {
        let textEditor: vscode.TextEditor | undefined = vscode.window.activeTextEditor;
        if (!textEditor) {
            return false;
        }

        const config: vscode.WorkspaceConfiguration = vscode.workspace.getConfiguration("launch", textEditor.document.uri);
        const folder: vscode.WorkspaceFolder | undefined = vscode.workspace.getWorkspaceFolder(textEditor.document.uri);
        const values: any[] | undefined = config.get<any[]>("configurations");
        if (!folder || !values) {
            return false;
        }

        const picks: Array<vscode.QuickPickItem> = [];
        for (const value of values) {
            if (!value.name || !value.request) {
                continue;
            }
            picks.push({
                label: value.name,
                detail: value.request,
            });
        }
        if (picks.length <= 0) {
            return false;
        }

        let launch: string = picks[0].label;
        if (picks.length > 1) { // pick one
            const choice: vscode.QuickPickItem | undefined = await vscode.window.showQuickPick(picks, {
                placeHolder: "Please choose a launch configuration for your debug session.  (Press ESC to cancel)",
                ignoreFocusOut: true,
            });
            if (!choice) {
                return false;
            }
            launch = choice.label;
        }

        return await vscode.debug.startDebugging(folder, launch);
    }
}

export const leetCodeDebugger: LeetCodeDebugger = new LeetCodeDebugger();
