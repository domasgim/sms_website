<?php

use App\Http\Controllers\SentMessageController;
use App\Http\Controllers\ReceiveMessageController;
use App\SentMessage;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| Web Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| contains the "web" middleware group. Now create something great!
|
*/

Route::get('/', function () {
    return view('project');
});

Route::get('/receivedMessages', 'ReceivedMessageController@index');
Route::get('/sentMessages', 'SentMessageController@index');
Route::post('/sentMessages', 'SentMessageController@store');
#Route::post('/sentMessages', 'SentMessageController@send_zmq');
Route::put('/receivedMessages/{receivedMessage}', 'ReceivedMessageController@update');
