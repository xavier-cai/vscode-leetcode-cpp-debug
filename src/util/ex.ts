import * as vscode from 'vscode'

export interface IQuickItemEx<T> extends vscode.QuickPickItem {
    value: T;
}