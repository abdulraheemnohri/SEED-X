package org.seedx.agent;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

import androidx.core.app.NotificationCompat;
import androidx.work.Worker;
import androidx.work.WorkerParameters;

/** Periodic reminder only. It never starts the agent, calls a provider, or executes tools. */
public final class SessionReminderWorker extends Worker {
    private static final String CHANNEL = "seedx_task_reminders";
    public SessionReminderWorker(Context context, WorkerParameters params) { super(context, params); }

    @Override public Result doWork() {
        Context context = getApplicationContext();
        if (Build.VERSION.SDK_INT >= 33 &&
            context.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            return Result.success();
        }
        NotificationManager manager = context.getSystemService(NotificationManager.class);
        if (manager == null) return Result.success();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(CHANNEL, "SEED-X task reminders", NotificationManager.IMPORTANCE_DEFAULT);
            channel.setDescription("User-scheduled reminders to review the SEED-X workspace.");
            manager.createNotificationChannel(channel);
        }
        Notification notification = new NotificationCompat.Builder(context, CHANNEL)
            .setSmallIcon(R.drawable.seedx_logo)
            .setContentTitle("SEED-X reminder")
            .setContentText("Review your tasks and agent session. No work was run automatically.")
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setAutoCancel(true)
            .build();
        manager.notify(2401, notification);
        return Result.success();
    }
}
