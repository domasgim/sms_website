<?php

namespace App\Http\Controllers;

use App\ReceivedMessage;
use Illuminate\Http\Request;

class ReceivedMessageController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        return [
            'received_messages' => ReceivedMessage::orderBy('created_at', 'desc')->get()
        ];
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        //
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        //
    }

    /**
     * Display the specified resource.
     *
     * @param  \App\ReceivedMessage  $receivedMessage
     * @return \Illuminate\Http\Response
     */
    public function show(ReceivedMessage $receivedMessage)
    {
        //
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  \App\ReceivedMessage  $receivedMessage
     * @return \Illuminate\Http\Response
     */
    public function edit(ReceivedMessage $receivedMessage)
    {
        //
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \App\ReceivedMessage  $receivedMessage
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, ReceivedMessage $receivedMessage)
    {
        $receivedMessage->update(['read' => $request['read']]);

        return [
            'message' => 'Message status updated!'
        ];
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  \App\ReceivedMessage  $receivedMessage
     * @return \Illuminate\Http\Response
     */
    public function destroy(ReceivedMessage $receivedMessage)
    {
        //
    }
}
